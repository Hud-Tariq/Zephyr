#ifndef FORECASTSECTION_H
#define FORECASTSECTION_H

#include <QWidget>
#include <QJsonArray>
#include <QJsonObject>

class QHBoxLayout;
class QScrollArea;
class QPushButton;

class ForecastSection : public QWidget {
    Q_OBJECT

public:
    explicit ForecastSection(QWidget *parent = nullptr);
    void updateForecastData(const QJsonArray &forecastData);

    // These methods need to be public since MainWindow is calling them directly
    void clearForecasts();
    void addDayForecast(const QString &day, const QString &iconCode, const QString &highTemp, const QString &lowTemp);

private:
    QScrollArea *m_scrollArea;
    QWidget *m_forecastCardsContainer;
    QHBoxLayout *m_forecastLayout;
};

#endif // FORECASTSECTION_H
