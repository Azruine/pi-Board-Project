#include "WeeklyWeatherModel.h"
#include <QHash>
#include <QByteArray>

WeeklyWeatherModel::WeeklyWeatherModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WeeklyWeatherModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_weatherList.count();
}

QVariant WeeklyWeatherModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_weatherList.count())
        return QVariant();

    WeatherData *weather = m_weatherList.at(index.row());
    switch (role) {
    case DayRole:
        return m_days.at(index.row());
    case TemperatureRole:
        return weather->temperature();
    case ConditionRole:
        return weather->condition();
    case IconUrlRole:
        return weather->iconUrl();
    case MaxTempRole:
        return weather->maxTemperature();
    case MinTempRole:
        return weather->minTemperature();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> WeeklyWeatherModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[DayRole] = "day";
    roles[TemperatureRole] = "temperature";
    roles[ConditionRole] = "condition";
    roles[IconUrlRole] = "iconUrl";
    roles[MaxTempRole] = "maxTemperature";
    roles[MinTempRole] = "minTemperature";
    return roles;
}

void WeeklyWeatherModel::addWeatherData(const QString &day, WeatherData *data) {
    beginInsertRows(QModelIndex(), m_weatherList.count(), m_weatherList.count());
    m_weatherList.append(data);
    m_days.append(day);
    endInsertRows();
}

void WeeklyWeatherModel::clear() {
    beginResetModel();
    qDeleteAll(m_weatherList);
    m_weatherList.clear();
    m_days.clear();
    endResetModel();
}
