#include "WeatherDataManager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDate>
#include <QDateTime>
#include <QLocale>
#include <QDebug>
#include <QFile>

WeatherDataManager::WeatherDataManager(QObject *parent)
    : QObject(parent)
{
    // conditions.json 파일을 로드하여 매핑 생성
    loadConditionMapping();

    // 초기 오늘 날씨 (테스트용 기본값)
    m_todayWeatherData = new WeatherData("영상 1도", "맑음", "", this);

    // 초기 주간 날씨 (하드코딩된 테스트값; 실제 업데이트 시 clear() 후 새로 추가)
    m_weeklyWeatherModel = new WeeklyWeatherModel(this);
    m_weeklyWeatherModel->addWeatherData("금", new WeatherData("영상 0도", "맑음", "", m_weeklyWeatherModel));
    m_weeklyWeatherModel->addWeatherData("토", new WeatherData("영하 1도", "눈", "", m_weeklyWeatherModel));
    m_weeklyWeatherModel->addWeatherData("일", new WeatherData("영상 1도", "비", "", m_weeklyWeatherModel));
    m_weeklyWeatherModel->addWeatherData("월", new WeatherData("영하 3도", "맑음", "", m_weeklyWeatherModel));
    m_weeklyWeatherModel->addWeatherData("화", new WeatherData("영하 2도", "맑음", "", m_weeklyWeatherModel));
    m_weeklyWeatherModel->addWeatherData("수", new WeatherData("영하 1도", "맑음", "", m_weeklyWeatherModel));
    m_weeklyWeatherModel->addWeatherData("목", new WeatherData("영상 0도", "맑음", "", m_weeklyWeatherModel));
}

WeeklyWeatherModel* WeatherDataManager::weeklyWeatherModel() const {
    return m_weeklyWeatherModel;
}

WeatherData* WeatherDataManager::todayWeatherData() const {
    return m_todayWeatherData;
}

void WeatherDataManager::loadConditionMapping()
{
    QFile file(":/json/conditions.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open conditions.json";
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        qWarning() << "conditions.json is not an array";
        return;
    }

    QJsonArray arr = doc.array();
    // m_conditionMapping 타입: QHash<int, QPair<QPair<QString, QString>, QString>>
    // 각 조건 코드에 대해 ((day_text, night_text), iconUrl)를 저장합니다.
    for (const QJsonValue &val : arr) {
        if (val.isObject()) {
            QJsonObject obj = val.toObject();
            int code = obj.value("code").toInt();

            // languages 배열에서 lang_name이 "Korean"인 항목을 찾음
            QString koreanDayText;
            QString koreanNightText;
            QJsonArray langs = obj.value("languages").toArray();
            for (const QJsonValue &langVal : langs) {
                QJsonObject langObj = langVal.toObject();
                if (langObj.value("lang_name").toString().compare("Korean", Qt::CaseInsensitive) == 0) {
                    koreanDayText = langObj.value("day_text").toString();
                    koreanNightText = langObj.value("night_text").toString();
                    break;
                }
            }
            // icon 필드: 예) "//cdn.weatherapi.com/weather/64x64/day/113.png"
            QString iconUrl = obj.value("icon").toString();
            if (iconUrl.startsWith("//"))
                iconUrl.prepend("https:");
            m_conditionMapping.insert(code, qMakePair(qMakePair(koreanDayText, koreanNightText), iconUrl));
        }
    }
}

void WeatherDataManager::updateWeather()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    // WeatherAPI forecast 엔드포인트 구성 (현재 날씨와 7일 예보 모두 포함)
    QString apiKey = "44c0db60ba0245309cd60250250302";
    QString city = "Seoul";
    QString urlString = QString("http://api.weatherapi.com/v1/forecast.json?key=%1&q=%2&days=7&lang=kr")
                            .arg(apiKey)
                            .arg(city);
    QUrl url(urlString);
    QNetworkRequest request(url);

    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            if (!jsonDoc.isNull() && jsonDoc.isObject()) {
                QJsonObject rootObj = jsonDoc.object();

                // [1] 오늘 날씨 업데이트 (current)
                QJsonObject currentObj = rootObj.value("current").toObject();
                double tempC = currentObj.value("temp_c").toDouble();
                QString tempStr = QString::number(tempC, 'f', 1) + "°C";
                int isDay = currentObj.value("is_day").toInt();

                QJsonObject condObj = currentObj.value("condition").toObject();
                int currentCode = condObj.value("code").toInt();
                QString conditionKorean;
                if (m_conditionMapping.contains(currentCode)) {
                    QPair<QPair<QString, QString>, QString> mapping = m_conditionMapping.value(currentCode);
                    // is_day가 1이면 day_text, 0이면 night_text 선택
                    conditionKorean = (isDay == 1) ? mapping.first.first : mapping.first.second;
                } else {
                    conditionKorean = condObj.value("text").toString();
                }
                QString currentIconUrl = condObj.value("icon").toString();
                if (currentIconUrl.startsWith("//"))
                    currentIconUrl.prepend("https:");

                m_todayWeatherData->updateData(tempStr, conditionKorean, currentIconUrl);

                // [2] 주간 예보 업데이트 (forecast)
                QJsonObject forecastObj = rootObj.value("forecast").toObject();
                QJsonArray forecastDays = forecastObj.value("forecastday").toArray();

                m_weeklyWeatherModel->clear();

                // 오늘 날짜 기준으로 내일부터 시작하도록: forecastday의 첫 번째 항목은 내일
                QLocale koreanLocale(QLocale::Korean);
                for (int i = 0; i < forecastDays.size(); ++i) {
                    // 오늘 기준으로 i+1일 후의 날짜 계산
                    QDate forecastDate = QDate::currentDate().addDays(i + 1);
                    // "dddd"로 전체 요일 이름을 한글로 표시 (예: "화요일")
                    QLocale koreanLocale(QLocale::Korean);
                    QString dayFull = koreanLocale.toString(forecastDate, "dddd");

                    QJsonObject dayForecast = forecastDays.at(i).toObject();
                    QJsonObject dayObj = dayForecast.value("day").toObject();
                    double maxTempC = dayObj.value("maxtemp_c").toDouble();
                    double minTempC = dayObj.value("mintemp_c").toDouble();
                    QString maxTempStr = QString::number(maxTempC, 'f', 1) + "°C";
                    QString minTempStr = QString::number(minTempC, 'f', 1) + "°C";

                    QJsonObject dayCondObj = dayObj.value("condition").toObject();
                    int dayCode = dayCondObj.value("code").toInt();
                    QString dayConditionKorean;
                    if (m_conditionMapping.contains(dayCode)) {
                        QPair<QPair<QString, QString>, QString> mapping = m_conditionMapping.value(dayCode);
                        // 주간 예보는 보통 낮의 condition을 사용
                        dayConditionKorean = mapping.first.first;
                    } else {
                        dayConditionKorean = dayCondObj.value("text").toString();
                    }
                    QString dayIconUrl = dayCondObj.value("icon").toString();
                    if (dayIconUrl.startsWith("//"))
                        dayIconUrl.prepend("https:");

                    m_weeklyWeatherModel->addWeatherData(dayFull,
                                                         new WeatherData(maxTempStr, minTempStr, dayConditionKorean, dayIconUrl, m_weeklyWeatherModel));
                }

                // [3] 향후 6시간 기온 업데이트 (hourly forecast)
                qint64 nowEpoch = QDateTime::currentSecsSinceEpoch();
                m_hourlyForecast.clear();
                int count = 0;
                // forecastDays를 순회하여 hour 배열에서 현재 이후 6시간 데이터 수집
                for (int i = 0; i < forecastDays.size() && count < 6; ++i) {
                    QJsonObject dayForecast = forecastDays.at(i).toObject();
                    QJsonArray hourArray = dayForecast.value("hour").toArray();
                    for (int j = 0; j < hourArray.size() && count < 6; ++j) {
                        QJsonObject hourObj = hourArray.at(j).toObject();
                        qint64 hourEpoch = hourObj.value("time_epoch").toVariant().toLongLong();
                        if (hourEpoch >= nowEpoch) {
                            QString timeStr = hourObj.value("time").toString(); // "YYYY-MM-DD HH:mm"
                            QString displayTime = timeStr.split(" ")[1];  // "HH:mm"
                            double hourTemp = hourObj.value("temp_c").toDouble();
                            QString hourTempStr = QString::number(hourTemp, 'f', 1) + "°C";

                            QJsonObject hourCondObj = hourObj.value("condition").toObject();
                            QString hourIcon = hourCondObj.value("icon").toString();
                            if (hourIcon.startsWith("//"))
                                hourIcon.prepend("https:");

                            QVariantMap hourForecast;
                            hourForecast["time"] = displayTime;
                            hourForecast["temp"] = hourTempStr;
                            hourForecast["icon"] = hourIcon;

                            m_hourlyForecast.append(hourForecast);
                            count++;
                        }
                    }
                }
                emit hourlyForecastChanged();
            } else {
                qDebug() << "유효하지 않은 JSON 데이터";
            }
        } else {
            qDebug() << "네트워크 에러:" << reply->errorString();
        }
        reply->deleteLater();
    });
}
