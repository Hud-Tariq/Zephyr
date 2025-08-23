#include "forecastssection.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QScrollArea>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QIcon>
#include <QFrame>
#include <QLinearGradient>
#include <QPainter>

ForecastsSection::ForecastsSection(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Section header with dark theme
    QWidget *headerWidget = new QWidget;
    headerWidget->setObjectName("forecastsSectionHeader");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(24, 16, 24, 16);
    headerLayout->setSpacing(16);

    QLabel *title = new QLabel("Extended Forecasts");
    title->setObjectName("forecastsSectionTitle");
    headerLayout->addWidget(title);

    headerLayout->addStretch();

    QLabel *infoLabel = new QLabel("Powered by OpenWeatherMap");
    infoLabel->setObjectName("forecastsSectionSubtitle");
    headerLayout->addWidget(infoLabel);

    layout->addWidget(headerWidget);

    // Tab widget with dark styling
    QTabWidget *forecastTabs = new QTabWidget;
    forecastTabs->setObjectName("forecastTabs");
    forecastTabs->setTabPosition(QTabWidget::North);
    forecastTabs->setTabShape(QTabWidget::Rounded);

    // Main scrollable container
    QScrollArea *mainScrollArea = new QScrollArea;
    mainScrollArea->setObjectName("forecastScrollArea");
    mainScrollArea->setWidgetResizable(true);
    mainScrollArea->setFrameShape(QFrame::NoFrame);
    mainScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *scrollContent = new QWidget;
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 24, 0, 24);
    scrollLayout->setSpacing(24);

    // 10-day forecast container with dark theme
    QWidget *tenDayContainer = new QWidget;
    tenDayContainer->setObjectName("forecastContainer");
    tenDayContainer->setMinimumHeight(400); // This container is styled by mainwindow.cpp now
    QVBoxLayout* tenDayContainerLayout = new QVBoxLayout(tenDayContainer);
    tenDayContainerLayout->setContentsMargins(0, 0, 0, 0);
    tenDayContainerLayout->setSpacing(0);

    // 10-day header
    QWidget *tenDayHeader = new QWidget;
    tenDayHeader->setObjectName("cardHeader");
    QHBoxLayout *tenDayHeaderLayout = new QHBoxLayout(tenDayHeader);
    tenDayHeaderLayout->setContentsMargins(24, 20, 24, 16);
    tenDayHeaderLayout->setSpacing(12);

    QLabel* tenDayTitle = new QLabel("10-Day Outlook");
    tenDayTitle->setObjectName("cardTitle");
    tenDayHeaderLayout->addWidget(tenDayTitle);

    tenDayHeaderLayout->addStretch();

    QLabel* trendIndicator = new QLabel("📈 Trending warmer");
    trendIndicator->setObjectName("trendIndicator");
    tenDayHeaderLayout->addWidget(trendIndicator);

    tenDayContainerLayout->addWidget(tenDayHeader);

    // Forecast content widget
    QWidget *tenDayForecastWidget = new QWidget;
    m_tenDayLayout = new QVBoxLayout(tenDayForecastWidget);
    m_tenDayLayout->setSpacing(0);
    m_tenDayLayout->setContentsMargins(24, 0, 24, 24);

    tenDayContainerLayout->addWidget(tenDayForecastWidget);
    scrollLayout->addWidget(tenDayContainer);

    // Enhanced chart container with premium dark styling
    QWidget *trendChart = new QWidget;
    trendChart->setObjectName("chartContainer");
    trendChart->setMinimumHeight(480); // This container is styled by mainwindow.cpp now
    QVBoxLayout *chartLayout = new QVBoxLayout(trendChart);
    chartLayout->setContentsMargins(0, 0, 0, 0);
    chartLayout->setSpacing(0);

    // Chart header
    QWidget *chartHeader = new QWidget;
    chartHeader->setObjectName("cardHeader");
    QHBoxLayout *chartHeaderLayout = new QHBoxLayout(chartHeader);
    chartHeaderLayout->setContentsMargins(24, 20, 24, 16);
    chartHeaderLayout->setSpacing(12);

    QVBoxLayout *chartTitleLayout = new QVBoxLayout;
    chartTitleLayout->setSpacing(4);

    QLabel *chartTitle = new QLabel("Temperature Overview");
    chartTitle->setObjectName("cardTitle");
    chartTitleLayout->addWidget(chartTitle);

    QLabel *chartSubtitle = new QLabel("Daily temperature trends with gradient visualization");
    chartSubtitle->setObjectName("cardSubtitle");
    chartTitleLayout->addWidget(chartSubtitle);

    chartHeaderLayout->addLayout(chartTitleLayout);
    chartHeaderLayout->addStretch();

    QWidget *periodSelector = new QWidget;
    periodSelector->setObjectName("periodSelector"); // Styled by mainwindow.cpp now
    QHBoxLayout *periodLayout = new QHBoxLayout(periodSelector);
    periodLayout->setContentsMargins(0, 0, 0, 0);
    periodLayout->setSpacing(0);

    QLabel *chartPeriod = new QLabel("10 Days");
    chartPeriod->setObjectName("activePeriod");
    periodLayout->addWidget(chartPeriod);

    chartHeaderLayout->addWidget(periodSelector);
    chartLayout->addWidget(chartHeader);

    // Chart area with premium dark styling
    QWidget *chartArea = new QWidget;
    chartArea->setObjectName("chartArea"); // Styled by mainwindow.cpp now
    QVBoxLayout *chartAreaLayout = new QVBoxLayout(chartArea);
    chartAreaLayout->setContentsMargins(24, 0, 24, 32);
    chartAreaLayout->setSpacing(0);

    // Create area series for gradient fill effect
    m_highSeries = new QLineSeries();
    m_highSeries->setName("High");
    QPen highPen(QColor("#fbbf24"), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    m_highSeries->setPen(highPen);

    m_lowSeries = new QLineSeries();
    m_lowSeries->setName("Low");
    QPen lowPen(QColor("#60a5fa"), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    m_lowSeries->setPen(lowPen);

    // Create area series for gradient fill
    m_areaSeries = new QAreaSeries(m_highSeries, m_lowSeries);
    m_areaSeries->setName("Temperature Range");

    // Create gradient brush for area fill
    QLinearGradient gradient(0, 0, 0, 1);
    gradient.setColorAt(0.0, QColor(251, 191, 36, 180));  // #fbbf24 with opacity
    gradient.setColorAt(0.3, QColor(217, 119, 6, 140));   // #d97706 with opacity
    gradient.setColorAt(0.7, QColor(146, 64, 14, 100));   // #92400e with opacity
    gradient.setColorAt(1.0, QColor(69, 26, 3, 60));      // #451a03 with opacity
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    QBrush areaBrush(gradient);
    m_areaSeries->setBrush(areaBrush);
    m_areaSeries->setPen(QPen(Qt::transparent));

    QChart *chart = new QChart();
    chart->addSeries(m_areaSeries);
    chart->addSeries(m_highSeries);
    chart->addSeries(m_lowSeries);

    // Dark theme chart styling
    chart->setBackgroundBrush(QBrush(Qt::transparent));
    chart->setPlotAreaBackgroundBrush(QBrush(Qt::transparent));
    chart->setMargins(QMargins(16, 16, 16, 16));
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setMarkerShape(QLegend::MarkerShapeRectangle);
    chart->legend()->setLabelColor(QColor("#94a3b8"));
    chart->legend()->setBackgroundVisible(false);
    chart->legend()->setBorderColor(Qt::transparent);

    QCategoryAxis *axisX = new QCategoryAxis();
    axisX->setLabelsAngle(-45);
    axisX->setGridLineVisible(true);
    axisX->setGridLineColor(QColor("#334155"));
    axisX->setGridLinePen(QPen(QColor("#334155"), 1, Qt::DashLine));
    axisX->setLineVisible(false);
    axisX->setLabelsColor(QColor("#94a3b8"));
    axisX->setTitleBrush(QBrush(QColor("#94a3b8")));

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d°");
    axisY->setGridLineVisible(true);
    axisY->setGridLineColor(QColor("#334155"));
    axisY->setGridLinePen(QPen(QColor("#334155"), 1, Qt::DashLine));
    axisY->setLabelsColor(QColor("#94a3b8"));
    axisY->setLineVisible(false);
    axisY->setTitleBrush(QBrush(QColor("#94a3b8")));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    m_highSeries->attachAxis(axisX);
    m_highSeries->attachAxis(axisY);
    m_lowSeries->attachAxis(axisX);
    m_lowSeries->attachAxis(axisY);
    m_areaSeries->attachAxis(axisX);
    m_areaSeries->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setObjectName("chartView");
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setBackgroundBrush(QBrush(Qt::transparent));
    chartView->setMinimumHeight(360);

    chartAreaLayout->addWidget(chartView);
    chartLayout->addWidget(chartArea);
    scrollLayout->addWidget(trendChart);

    mainScrollArea->setWidget(scrollContent);
    forecastTabs->addTab(mainScrollArea, "📅 Daily Forecast");
    layout->addWidget(forecastTabs, 1);
}

void ForecastsSection::updateForecasts(const QJsonArray &dailyForecasts) {
    if (!m_tenDayLayout || !m_highSeries || !m_lowSeries || !m_areaSeries) return;

    // Clear old data
    QLayoutItem *child;
    while ((child = m_tenDayLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    m_highSeries->clear();
    m_lowSeries->clear();

    m_tenDayLayout->addStretch(1); // For vertical centering

    double minTemp = 100, maxTemp = -100;

    // Icon mapping
    QMap<QString, QString> iconMap;
    iconMap["01d"] = "sunny.svg"; iconMap["01n"] = "sunny.svg";
    iconMap["02d"] = "partly-cloudy.svg"; iconMap["02n"] = "partly-cloudy.svg";
    iconMap["03d"] = "cloudy.svg"; iconMap["03n"] = "cloudy.svg";
    iconMap["04d"] = "cloudy.svg"; iconMap["04n"] = "cloudy.svg";
    iconMap["09d"] = "rainy.svg"; iconMap["09n"] = "rainy.svg";
    iconMap["10d"] = "rainy.svg"; iconMap["10n"] = "rainy.svg";
    iconMap["11d"] = "thunderstorm.svg"; iconMap["11n"] = "thunderstorm.svg";
    iconMap["13d"] = "snow.svg"; iconMap["13n"] = "snow.svg";
    iconMap["50d"] = "fog.svg"; iconMap["50n"] = "fog.svg";

    // Process forecast data
    for (int i = 0; i < dailyForecasts.size(); ++i) {
        QJsonObject dayData = dailyForecasts[i].toObject();
        QDateTime date = QDateTime::fromSecsSinceEpoch(dayData.value("dt").toInteger());
        double high = dayData.value("temp").toObject().value("max").toDouble();
        double low = dayData.value("temp").toObject().value("min").toDouble();
        QString condition = dayData.value("weather").toArray()[0].toObject().value("description").toString();
        condition[0] = condition[0].toUpper();
        double precip = dayData.value("pop").toDouble() * 100;
        QString iconCode = dayData.value("weather").toArray()[0].toObject().value("icon").toString();
        double humidity = dayData.value("humidity").toDouble();
        double windSpeed = dayData.value("speed").toDouble();

        if (low < minTemp) minTemp = low;
        if (high > maxTemp) maxTemp = high;

        // Day row widget with dark theme
        QWidget *dayRow = new QWidget;
        dayRow->setObjectName("forecastRow");
        dayRow->setProperty("isToday", i == 0);
        dayRow->setProperty("rowIndex", i);

        QHBoxLayout *rowLayout = new QHBoxLayout(dayRow);
        rowLayout->setContentsMargins(0, 16, 0, 16);
        rowLayout->setSpacing(20);

        // Date column with dark styling
        QWidget *dateColumn = new QWidget;
        dateColumn->setObjectName("dateColumn");
        dateColumn->setFixedWidth(85);
        QVBoxLayout *dayLayout = new QVBoxLayout(dateColumn);
        dayLayout->setContentsMargins(0, 0, 0, 0);
        dayLayout->setSpacing(2);

        QLabel *dayLabel = new QLabel(i == 0 ? "Today" : date.toString("dddd"));
        dayLabel->setObjectName("forecastDay");

        QLabel *dateLabel = new QLabel(date.toString("MMM d"));
        dateLabel->setObjectName("forecastDate");

        dayLayout->addWidget(dayLabel);
        dayLayout->addWidget(dateLabel);

        // Icon column (unchanged)
        QWidget *iconColumn = new QWidget;
        iconColumn->setObjectName("iconColumn");
        iconColumn->setFixedWidth(56);
        QVBoxLayout *iconLayout = new QVBoxLayout(iconColumn);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        iconLayout->setAlignment(Qt::AlignCenter);

        QLabel *iconLabel = new QLabel;
        iconLabel->setObjectName("weatherIcon");
        QString iconPath = "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/weather/" + iconMap.value(iconCode, "partly-cloudy.svg");
        iconLabel->setPixmap(QIcon(iconPath).pixmap(44, 44));
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setFixedSize(56, 56);

        iconLayout->addWidget(iconLabel);

        // Condition column with dark styling
        QWidget *conditionColumn = new QWidget;
        conditionColumn->setObjectName("conditionColumn");
        QVBoxLayout *conditionLayout = new QVBoxLayout(conditionColumn);
        conditionLayout->setContentsMargins(0, 0, 0, 0);
        conditionLayout->setSpacing(4);

        QLabel *conditionLabel = new QLabel(condition);
        conditionLabel->setObjectName("forecastCondition");
        conditionLabel->setWordWrap(true);

        QWidget *detailsWidget = new QWidget;
        QHBoxLayout *detailsLayout = new QHBoxLayout(detailsWidget);
        detailsLayout->setContentsMargins(0, 0, 0, 0);
        detailsLayout->setSpacing(12);

        QLabel *humidityLabel = new QLabel(QString("💧 %1%").arg(QString::number(humidity, 'f', 0)));
        humidityLabel->setObjectName("weatherDetail");

        QLabel *windLabel = new QLabel(QString("💨 %1 m/s").arg(QString::number(windSpeed, 'f', 1)));
        windLabel->setObjectName("weatherDetail");

        detailsLayout->addWidget(humidityLabel);
        detailsLayout->addWidget(windLabel);
        detailsLayout->addStretch();

        conditionLayout->addWidget(conditionLabel);
        conditionLayout->addWidget(detailsWidget);

        // Precipitation column with enhanced styling
        QWidget *precipColumn = new QWidget;
        precipColumn->setObjectName("precipColumn");
        precipColumn->setFixedWidth(60);
        QVBoxLayout *precipLayout = new QVBoxLayout(precipColumn);
        precipLayout->setContentsMargins(0, 0, 0, 0);
        precipLayout->setAlignment(Qt::AlignCenter);

        QLabel *precipLabel = new QLabel(QString::number(precip, 'f', 0) + "%");
        precipLabel->setObjectName("precipChance");
        precipLabel->setAlignment(Qt::AlignCenter);

        precipLabel->setProperty("precipLevel", precip >= 70 ? "high" : (precip >= 30 ? "medium" : "low"));

        precipLayout->addWidget(precipLabel);

        // Temperature column with enhanced styling
        QWidget *tempColumn = new QWidget;
        tempColumn->setObjectName("tempColumn");
        tempColumn->setFixedWidth(120);
        QVBoxLayout *tempColumnLayout = new QVBoxLayout(tempColumn);
        tempColumnLayout->setContentsMargins(0, 0, 0, 0);
        tempColumnLayout->setSpacing(8);

        QHBoxLayout *tempLayout = new QHBoxLayout;
        tempLayout->setContentsMargins(0, 0, 0, 0);
        tempLayout->setSpacing(8);

        QLabel *lowLabel = new QLabel(QString::number(qRound(low)) + "°");
        lowLabel->setObjectName("tempLow");
        lowLabel->setAlignment(Qt::AlignCenter);
        lowLabel->setFixedWidth(35);

        QWidget *tempBar = new QWidget;
        tempBar->setObjectName("tempRangeBar");
        tempBar->setFixedHeight(8);
        tempBar->setMinimumWidth(40);

        QLabel *highLabel = new QLabel(QString::number(qRound(high)) + "°");
        highLabel->setObjectName("tempHigh");
        highLabel->setAlignment(Qt::AlignCenter);
        highLabel->setFixedWidth(35);

        tempLayout->addWidget(lowLabel);
        tempLayout->addWidget(tempBar, 1);
        tempLayout->addWidget(highLabel);
        tempColumnLayout->addLayout(tempLayout);

        rowLayout->addWidget(dateColumn, 0, Qt::AlignVCenter);
        rowLayout->addWidget(iconColumn, 0, Qt::AlignVCenter);
        rowLayout->addWidget(conditionColumn, 1, Qt::AlignVCenter);
        rowLayout->addWidget(precipColumn, 0, Qt::AlignVCenter);
        rowLayout->addWidget(tempColumn, 0, Qt::AlignVCenter);

        m_tenDayLayout->addWidget(dayRow);

        // Add separator between rows with dark styling
        if (i < dailyForecasts.size() - 1) {
            QFrame *separator = new QFrame;
            separator->setObjectName("rowSeparator");
            separator->setFrameShape(QFrame::HLine);
            m_tenDayLayout->addWidget(separator);
        }

        // Add data to chart
        m_highSeries->append(i, high);
        m_lowSeries->append(i, low);
    }

    m_tenDayLayout->addStretch(1); // For vertical centering

    // Update chart axes with enhanced dark styling
    QChart *chart = m_highSeries->chart();
    if (!chart->axes(Qt::Horizontal).isEmpty()) chart->removeAxis(chart->axes(Qt::Horizontal).first());

    QCategoryAxis *axisX = new QCategoryAxis();
    axisX->setLabelsAngle(-45);
    axisX->setGridLineVisible(true);
    axisX->setGridLineColor(QColor("#334155"));
    axisX->setGridLinePen(QPen(QColor("#334155"), 1, Qt::DashLine));
    axisX->setLineVisible(false);
    axisX->setLabelsColor(QColor("#94a3b8"));

    for (int i = 0; i < dailyForecasts.size(); ++i) {
        QDateTime date = QDateTime::fromSecsSinceEpoch(dailyForecasts[i].toObject().value("dt").toInteger());
        QString label = i == 0 ? "Today" : date.toString("ddd d");
        axisX->append(label, i);
    }

    chart->addAxis(axisX, Qt::AlignBottom);
    m_highSeries->attachAxis(axisX);
    m_lowSeries->attachAxis(axisX);
    m_areaSeries->attachAxis(axisX);

    QValueAxis *axisY = static_cast<QValueAxis*>(chart->axes(Qt::Vertical)[0]);
    axisY->setRange(floor(minTemp - 5), ceil(maxTemp + 5));
    axisY->setGridLineColor(QColor("#334155"));
    axisY->setGridLinePen(QPen(QColor("#334155"), 1, Qt::DashLine));
    axisY->setGridLineVisible(true);
    axisY->setLabelsColor(QColor("#94a3b8"));
    axisY->setLineVisible(false);
    axisY->setTickCount(8);
}
