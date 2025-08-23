#include "weathercard.h"
#include <QDate>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

WeatherCard::WeatherCard(QWidget *parent) : QWidget(parent) {
    setObjectName("weatherCard");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);

    // Left Column - Temperature and conditions
    QVBoxLayout *leftCol = new QVBoxLayout;

    tempLabel = new QLabel("--°");
    tempLabel->setObjectName("tempLabel");

    conditionLabel = new QLabel("--");
    conditionLabel->setObjectName("conditionLabel");
    conditionLabel->setWordWrap(true);
    conditionLabel->setMinimumHeight(30);

    QLabel *dateLabel = new QLabel(QDate::currentDate().toString("dddd, MMMM d"));
    dateLabel->setObjectName("dateLabel");

    leftCol->addWidget(tempLabel);
    leftCol->addWidget(conditionLabel);
    leftCol->addWidget(dateLabel);
    leftCol->addStretch();

    // Center - Weather icon
    conditionIcon = new QLabel;
    conditionIcon->setObjectName("conditionIcon");
    conditionIcon->setAlignment(Qt::AlignCenter);
    conditionIcon->setMinimumSize(150, 150);
    QPixmap weatherIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/sunny.svg");
    conditionIcon->setPixmap(weatherIcon.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Right Column - Additional info
    QVBoxLayout *rightCol = new QVBoxLayout;
    rightCol->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLabel *highLowLabel = new QLabel("H: --° L: --°");
    highLowLabel->setObjectName("highLowLabel");
    highLowLabel->setAlignment(Qt::AlignRight);

    // Sunset/Sunrise info
    QHBoxLayout *sunInfoLayout = new QHBoxLayout;

    QLabel *sunriseIcon = new QLabel;
    sunriseIcon->setPixmap(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/sunrise.svg").pixmap(24, 24));

    sunriseLabel = new QLabel("--:--");
    sunriseLabel->setObjectName("sunTimeLabel");

    sunInfoLayout->addWidget(sunriseIcon);
    sunInfoLayout->addWidget(sunriseLabel);

    QHBoxLayout *sunsetLayout = new QHBoxLayout;

    QLabel *sunsetIcon = new QLabel;
    sunsetIcon->setPixmap(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/sunset.svg").pixmap(24, 24));

    sunsetLabel = new QLabel("--:--");
    sunsetLabel->setObjectName("sunTimeLabel");

    sunsetLayout->addWidget(sunsetIcon);
    sunsetLayout->addWidget(sunsetLabel);

    rightCol->addWidget(highLowLabel);
    rightCol->addLayout(sunInfoLayout);
    rightCol->addLayout(sunsetLayout);
    rightCol->addStretch();

    layout->addLayout(leftCol, 2);
    layout->addWidget(conditionIcon, 1);
    layout->addLayout(rightCol, 2);
}
