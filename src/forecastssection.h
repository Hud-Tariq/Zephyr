#ifndef FORECASTSSECTION_H
#define FORECASTSSECTION_H

#include <QWidget>
#include <QVBoxLayout>

// Forward declarations
class QJsonArray;
class QLineSeries;
class QAreaSeries;

class ForecastsSection : public QWidget
{
    Q_OBJECT

public:
    explicit ForecastsSection(QWidget *parent = nullptr);
    void updateForecasts(const QJsonArray &dailyForecasts);

private:
    QVBoxLayout *m_tenDayLayout;
    QLineSeries *m_highSeries;
    QLineSeries *m_lowSeries;
    QAreaSeries *m_areaSeries;  // New member for gradient area chart
};

#endif // FORECASTSSECTION_H
