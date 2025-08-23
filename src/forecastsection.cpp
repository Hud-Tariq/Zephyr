#include "forecastsection.h"
#include <QPushButton>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QSvgRenderer>
#include <QPainter>
#include <QPixmap>
#include <QScrollArea>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

// Helper function to create weather icon with proper styling
QPixmap createWeatherIcon(const QString &iconCode, const QSize &size) {
    QMap<QString, QString> iconMap;
    iconMap["01d"] = "clear-day.svg";
    iconMap["01n"] = "clear-night.svg";
    iconMap["02d"] = "partly-cloudy-day.svg";
    iconMap["02n"] = "partly-cloudy-night.svg";
    iconMap["03d"] = "cloudy.svg";
    iconMap["03n"] = "cloudy.svg";
    iconMap["04d"] = "overcast.svg";
    iconMap["04n"] = "overcast.svg";
    iconMap["09d"] = "drizzle.svg";
    iconMap["09n"] = "drizzle.svg";
    iconMap["10d"] = "rain.svg";
    iconMap["10n"] = "rain.svg";
    iconMap["11d"] = "thunderstorms.svg";
    iconMap["11n"] = "thunderstorms.svg";
    iconMap["13d"] = "snow.svg";
    iconMap["13n"] = "snow.svg";
    iconMap["50d"] = "fog.svg";
    iconMap["50n"] = "fog.svg";

    QString iconFile = iconMap.value(iconCode, "partly-cloudy-day.svg");
    QString iconPath = QString("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/weather/%1").arg(iconFile);

    QIcon icon(iconPath);
    if (!icon.isNull()) {
        return icon.pixmap(size);
    }

    QPixmap fallback(size);
    fallback.fill(Qt::transparent);
    QPainter painter(&fallback);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor("#7c9cff"));
    painter.setPen(Qt::NoPen);
    painter.drawRect(fallback.rect().adjusted(4, 4, -4, -4));
    return fallback;
}

ForecastSection::ForecastSection(QWidget *parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(20);

    // Header section with improved layout
    QWidget *headerWidget = new QWidget();
    headerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(12);

    // Title with icon
    QWidget *titleWidget = new QWidget();
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(12);

    // Calendar icon for forecast
    QLabel *titleIcon = new QLabel();
    titleIcon->setFixedSize(24, 24);
    titleIcon->setPixmap(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/calendar.svg").pixmap(24, 24));
    titleLayout->addWidget(titleIcon);

    QLabel *title = new QLabel("7-Day Forecast");
    title->setObjectName("sectionTitle");
    title->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    titleLayout->addWidget(title);

    headerLayout->addWidget(titleWidget);
    headerLayout->addStretch();

    mainLayout->addWidget(headerWidget);

    // Create main forecast container with background
    QWidget *forecastContainer = new QWidget();
    forecastContainer->setObjectName("forecastContainer");

    QVBoxLayout *containerLayout = new QVBoxLayout(forecastContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    m_scrollArea = new QScrollArea();
    m_scrollArea->setObjectName("forecastScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_scrollArea->setFixedHeight(160);
    m_scrollArea->setStyleSheet("background: transparent; border: none;");

    // Container for the day cards
    m_forecastCardsContainer = new QWidget();
    m_forecastCardsContainer->setObjectName("forecastCards");
    m_forecastCardsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_forecastCardsContainer->setStyleSheet("background: transparent;");

    // Create the layout for forecast cards
    m_forecastLayout = new QHBoxLayout(m_forecastCardsContainer);
    m_forecastLayout->setContentsMargins(0, 0, 0, 0);
    m_forecastLayout->setSpacing(12);
    m_forecastLayout->setAlignment(Qt::AlignCenter);

    m_scrollArea->setWidget(m_forecastCardsContainer);
    containerLayout->addWidget(m_scrollArea);

    mainLayout->addWidget(forecastContainer);
}

void ForecastSection::clearForecasts() {
    if (!m_forecastLayout) return;

    QLayoutItem *child;
    while ((child = m_forecastLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
}

void ForecastSection::addDayForecast(const QString &day, const QString &iconCode, const QString &highTemp, const QString &lowTemp) {
    if (!m_forecastLayout) return;

    QWidget *dayCard = new QWidget();
    dayCard->setObjectName("dayForecastCard");
    dayCard->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    dayCard->setFixedSize(100, 140);

    // No background or borders for inner cards
    dayCard->setStyleSheet("background: transparent; border: none;");

    QVBoxLayout *cardLayout = new QVBoxLayout(dayCard);
    cardLayout->setContentsMargins(8, 12, 8, 12);
    cardLayout->setSpacing(8);
    cardLayout->setAlignment(Qt::AlignCenter);

    // Day label
    QLabel *dayLabel = new QLabel(day);
    dayLabel->setObjectName("forecastDay");
    dayLabel->setAlignment(Qt::AlignCenter);
    dayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    dayLabel->setStyleSheet("background: transparent; font-weight: 500;");

    // Weather icon with perfect centering
    QLabel *iconLabel = new QLabel();
    iconLabel->setFixedSize(48, 48);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setPixmap(createWeatherIcon(iconCode, QSize(48, 48)));
    iconLabel->setStyleSheet("background: transparent;");

    // Temperature container with better spacing
    QWidget *tempContainer = new QWidget();
    tempContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *tempLayout = new QVBoxLayout(tempContainer);
    tempLayout->setContentsMargins(0, 0, 0, 0);
    tempLayout->setSpacing(1);
    tempLayout->setAlignment(Qt::AlignCenter);

    QLabel *highLabel = new QLabel(highTemp);
    highLabel->setObjectName("forecastHigh");
    highLabel->setAlignment(Qt::AlignCenter);
    highLabel->setStyleSheet("background: transparent; font-weight: 600;");

    QLabel *lowLabel = new QLabel(lowTemp);
    lowLabel->setObjectName("forecastLow");
    lowLabel->setAlignment(Qt::AlignCenter);
    lowLabel->setStyleSheet("background: transparent; font-weight: 400; opacity: 0.7;");

    tempLayout->addWidget(highLabel);
    tempLayout->addWidget(lowLabel);

    // Add all elements to card with perfect alignment
    cardLayout->addWidget(dayLabel, 0, Qt::AlignCenter);
    cardLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
    cardLayout->addWidget(tempContainer, 0, Qt::AlignCenter);

    m_forecastLayout->addWidget(dayCard);

    m_forecastCardsContainer->adjustSize();
}

void ForecastSection::updateForecastData(const QJsonArray &forecastData) {
    // Clear existing forecasts first
    clearForecasts();

    // Immediately add new data without waiting for animations
    for (int i = 0; i < forecastData.size() && i < 7; ++i) {
        QJsonObject dayData = forecastData[i].toObject();

        QString day = dayData["day"].toString();
        QString iconCode = dayData["icon"].toString();
        QString highTemp = QString("%1°").arg(dayData["temp_max"].toInt());
        QString lowTemp = QString("%1°").arg(dayData["temp_min"].toInt());

        addDayForecast(day, iconCode, highTemp, lowTemp);
    }
}
