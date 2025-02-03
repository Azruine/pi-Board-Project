#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "WeatherDataManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    WeatherDataManager weatherManager;

    engine.rootContext()->setContextProperty("todayWeatherData", weatherManager.todayWeatherData());
    engine.rootContext()->setContextProperty("weeklyWeatherModel", weatherManager.weeklyWeatherModel());
    engine.rootContext()->setContextProperty("weatherManager", &weatherManager);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(QUrl("qrc:/Main.qml"));

    weatherManager.updateWeather();

    return app.exec();
}
