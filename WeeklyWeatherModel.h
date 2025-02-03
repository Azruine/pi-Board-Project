#ifndef WEEKLYWEATHERMODEL_H
#define WEEKLYWEATHERMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include "WeatherData.h"

// 전역 enum: QML에 노출할 역할들을 정의합니다.
enum WeatherRoles {
    DayRole = Qt::UserRole + 1,
    TemperatureRole,
    ConditionRole,
    IconUrlRole,
    MaxTempRole,
    MinTempRole
};

class WeeklyWeatherModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit WeeklyWeatherModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 주간 날씨 데이터를 추가하는 메소드
    void addWeatherData(const QString &day, WeatherData *data);

    // 모델의 모든 데이터를 삭제하는 메소드
    void clear();

private:
    QList<WeatherData*> m_weatherList;
    QStringList m_days;
};

#endif // WEEKLYWEATHERMODEL_H
