#ifndef WEATHERDATAMANAGER_H
#define WEATHERDATAMANAGER_H

#include <QObject>
#include "WeeklyWeatherModel.h"
#include "WeatherData.h"
#include <QHash>
#include <QPair>
#include <QVariantList>

class WeatherDataManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList hourlyForecast READ hourlyForecast NOTIFY hourlyForecastChanged)
public:
    explicit WeatherDataManager(QObject *parent = nullptr);

    WeeklyWeatherModel* weeklyWeatherModel() const;
    WeatherData* todayWeatherData() const;

    // WeatherAPI를 통해 데이터를 받아와 업데이트하는 메소드
    Q_INVOKABLE void updateWeather();

    QVariantList hourlyForecast() const { return m_hourlyForecast; }

signals:
    void hourlyForecastChanged();

private:
    // conditions.json에서 로드한 매핑: key = condition code, value = ( (day_text, night_text), iconUrl )
    QHash<int, QPair<QPair<QString, QString>, QString>> m_conditionMapping;
    void loadConditionMapping();

    WeatherData* m_todayWeatherData;
    WeeklyWeatherModel* m_weeklyWeatherModel;

    QVariantList m_hourlyForecast; // 향후 6시간 기온 정보를 저장할 QVariantList
};

#endif // WEATHERDATAMANAGER_H
