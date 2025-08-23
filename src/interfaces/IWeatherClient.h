#pragma once
#include <QObject>
#include <QString>

class IWeatherClient : public QObject {
    Q_OBJECT
public:
    virtual void fetchWeatherData(const QString &city) = 0;
signals:
    void weatherDataReceived(const QJsonObject &data);
    void errorOccurred(const QString &message);
};
