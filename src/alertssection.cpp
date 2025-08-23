#include "alertssection.h"
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QJsonObject>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QIcon>
#include <QSvgRenderer>
#include <QPainter>
#include <QPixmap>
#include <QSizePolicy>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>

QPixmap createColoredIcon(const QString &path, const QSize &size, const QColor &color) {
    QSvgRenderer renderer(path);
    if (!renderer.isValid()) {
        QPixmap fallback(size);
        fallback.fill(color);
        return fallback;
    }

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter);

    // Apply color overlay more efficiently
    QPainter overlayPainter(&pixmap);
    overlayPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    overlayPainter.fillRect(pixmap.rect(), color);
    overlayPainter.end();

    return pixmap;
}

AlertCard::AlertCard(const QJsonObject &alertData, QWidget *parent)
    : QWidget(parent), m_expanded(false), m_expandAnimation(nullptr)
{
    setObjectName("alertCard");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QString event = alertData["event"].toString();
    QString description = alertData["description"].toString();
    QString source = alertData["sender_name"].toString();
    QString severity = alertData["severity"].toString().toLower();

    QDateTime startTime = QDateTime::fromSecsSinceEpoch(alertData["start"].toInteger());
    QDateTime endTime = QDateTime::fromSecsSinceEpoch(alertData["end"].toInteger());
    QString timeString = QString("From: %1 To: %2")
                             .arg(startTime.toString("MMM d, h:mm AP"))
                             .arg(endTime.toString("MMM d, h:mm AP"));

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Card container with proper sizing
    QWidget *cardContainer = new QWidget(this);
    cardContainer->setObjectName("alertCardContainer");
    cardContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Set severity-based styling
    if (severity == "severe" || severity == "extreme") {
        cardContainer->setProperty("alertSeverity", "severe");
    } else if (severity == "moderate") {
        cardContainer->setProperty("alertSeverity", "moderate");
    } else {
        cardContainer->setProperty("alertSeverity", "minor");
    }

    QVBoxLayout *cardLayout = new QVBoxLayout(cardContainer);
    cardLayout->setContentsMargins(20, 16, 20, 16);
    cardLayout->setSpacing(12);

    // Header section with proper alignment
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(16);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    // Alert icon with consistent sizing
    QLabel *alertIcon = new QLabel();
    alertIcon->setFixedSize(28, 28);
    alertIcon->setScaledContents(true);

    QColor iconColor;
    QString iconPath;
    if (severity == "severe" || severity == "extreme") {
        iconColor = QColor("#ff4d6a");
        iconPath = "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/warning.svg";
    } else if (severity == "moderate") {
        iconColor = QColor("#ffa726");
        iconPath = "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/alerts.svg";
    } else {
        iconColor = QColor("#7c9cff");
        iconPath = "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/info.svg";
    }

    alertIcon->setPixmap(createColoredIcon(iconPath, QSize(28, 28), iconColor));
    headerLayout->addWidget(alertIcon);

    // Title and source container with proper stretch
    QWidget *titleContainer = new QWidget();
    titleContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    QVBoxLayout *titleLayout = new QVBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);

    m_titleLabel = new QLabel(event);
    m_titleLabel->setObjectName("alertTitle");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QLabel *sourceLabel = new QLabel(QString("Source: %1").arg(source));
    sourceLabel->setObjectName("alertSource");
    sourceLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    titleLayout->addWidget(m_titleLabel);
    titleLayout->addWidget(sourceLabel);
    headerLayout->addWidget(titleContainer, 1);

    // Expand button with proper sizing and cursor
    m_expandButton = new QPushButton();
    m_expandButton->setObjectName("alertExpandButton");
    m_expandButton->setFixedSize(32, 32);
    m_expandButton->setIcon(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/chevron-down.svg"));
    m_expandButton->setCursor(Qt::PointingHandCursor);
    m_expandButton->setFocusPolicy(Qt::NoFocus);
    connect(m_expandButton, &QPushButton::clicked, this, &AlertCard::toggleExpand);

    headerLayout->addWidget(m_expandButton);
    cardLayout->addLayout(headerLayout);

    // Time information with proper spacing
    m_timeLabel = new QLabel(timeString);
    m_timeLabel->setObjectName("alertTime");
    m_timeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_timeLabel->setContentsMargins(44, 0, 0, 0); // Align with title text
    cardLayout->addWidget(m_timeLabel);

    // Expandable details container with proper setup
    m_detailsContainer = new QWidget();
    m_detailsContainer->setObjectName("alertDetailsContainer");
    m_detailsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_detailsContainer->setVisible(false);

    QVBoxLayout *detailsLayout = new QVBoxLayout(m_detailsContainer);
    detailsLayout->setContentsMargins(44, 8, 0, 0); // Align with title text
    detailsLayout->setSpacing(12);

    // Description with proper formatting
    m_descriptionLabel = new QLabel(description);
    m_descriptionLabel->setObjectName("alertDescription");
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_descriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    detailsLayout->addWidget(m_descriptionLabel);

    // Add action button for severe alerts with proper alignment
    if (severity == "severe" || severity == "extreme") {
        QHBoxLayout *actionLayout = new QHBoxLayout();
        actionLayout->setContentsMargins(0, 8, 0, 0);
        actionLayout->addStretch(); // Push button to the right

        QPushButton *actionButton = new QPushButton("Safety Information");
        actionButton->setObjectName("primaryButton");
        actionButton->setFixedWidth(160);
        actionButton->setCursor(Qt::PointingHandCursor);
        actionLayout->addWidget(actionButton);

        detailsLayout->addLayout(actionLayout);
    }

    cardLayout->addWidget(m_detailsContainer);
    mainLayout->addWidget(cardContainer);

    // Setup smooth animation
    m_expandAnimation = new QPropertyAnimation(m_detailsContainer, "maximumHeight", this);
    m_expandAnimation->setDuration(250);
    m_expandAnimation->setEasingCurve(QEasingCurve::OutQuart);

    // Initialize proper heights after widget is shown
    QTimer::singleShot(10, this, [this]() {
        m_detailsContainer->adjustSize();
        m_expandedHeight = m_detailsContainer->sizeHint().height();
        m_detailsContainer->setMaximumHeight(0);
    });
}

void AlertCard::toggleExpand() {
    setExpanded(!m_expanded);
}

void AlertCard::setExpanded(bool expanded) {
    if (m_expanded == expanded || !m_expandAnimation)
        return;

    m_expanded = expanded;

    // Ensure we have proper height calculation
    if (m_expandedHeight <= 0) {
        m_detailsContainer->adjustSize();
        m_expandedHeight = m_detailsContainer->sizeHint().height();
    }

    // Update button icon with smooth transition
    QString iconPath = m_expanded ?
                           "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/chevron-up.svg" :
                           "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/chevron-down.svg";
    m_expandButton->setIcon(QIcon(iconPath));

    // Stop any running animation
    if (m_expandAnimation->state() == QAbstractAnimation::Running) {
        m_expandAnimation->stop();
    }

    if (m_expanded) {
        m_detailsContainer->setVisible(true);
        m_expandAnimation->setStartValue(m_detailsContainer->maximumHeight());
        m_expandAnimation->setEndValue(m_expandedHeight);
    } else {
        m_expandAnimation->setStartValue(m_detailsContainer->maximumHeight());
        m_expandAnimation->setEndValue(0);

        // Hide container after animation completes
        connect(m_expandAnimation, &QPropertyAnimation::finished, this, [this]() {
            if (!m_expanded) {
                m_detailsContainer->setVisible(false);
            }
            disconnect(m_expandAnimation, &QPropertyAnimation::finished, this, nullptr);
        }, Qt::UniqueConnection);
    }

    m_expandAnimation->start();
}

// AlertsSection implementation
AlertsSection::AlertsSection(QWidget *parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(24);

    // Header with improved layout
    QWidget *headerWidget = new QWidget();
    headerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(16);

    // Title section with icon
    QWidget *titleWidget = new QWidget();
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(12);

    QLabel *titleIcon = new QLabel();
    titleIcon->setFixedSize(24, 24);
    titleIcon->setPixmap(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/bell.svg").pixmap(24, 24));
    titleLayout->addWidget(titleIcon);

    QLabel *title = new QLabel("Weather Alerts");
    title->setObjectName("sectionTitle");
    title->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    titleLayout->addWidget(title);

    headerLayout->addWidget(titleWidget);
    headerLayout->addStretch(); // Push count badge to the right

    // Alert count badge with proper styling
    m_alertCountLabel = new QLabel("0 Active");
    m_alertCountLabel->setObjectName("alertCountBadge");
    m_alertCountLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    headerLayout->addWidget(m_alertCountLabel);

    mainLayout->addWidget(headerWidget);

    // Create scroll area with proper configuration
    m_scrollArea = new QScrollArea();
    m_scrollArea->setObjectName("alertsScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Container for alerts with proper sizing
    m_alertsContainer = new QWidget();
    m_alertsContainer->setObjectName("alertsContainer");
    m_alertsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_alertsLayout = new QVBoxLayout(m_alertsContainer);
    m_alertsLayout->setContentsMargins(0, 0, 0, 0);
    m_alertsLayout->setSpacing(16);
    m_alertsLayout->addStretch(1); // Flexible stretch at the bottom

    m_scrollArea->setWidget(m_alertsContainer);
    mainLayout->addWidget(m_scrollArea, 1);

    // Create the "No alerts" view
    createNoAlertsView();

    // Initially show the "No alerts" view
    m_noAlertsContainer->setVisible(true);
    m_scrollArea->setVisible(false);
}

void AlertsSection::createNoAlertsView() {
    m_noAlertsContainer = new QWidget();
    m_noAlertsContainer->setObjectName("noAlertsContainer");
    m_noAlertsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *noAlertsLayout = new QVBoxLayout(m_noAlertsContainer);
    noAlertsLayout->setContentsMargins(40, 40, 40, 40);
    noAlertsLayout->setSpacing(20);
    noAlertsLayout->setAlignment(Qt::AlignCenter);

    // Icon with proper scaling
    QLabel *iconLabel = new QLabel();
    iconLabel->setFixedSize(64, 64);
    iconLabel->setScaledContents(true);
    iconLabel->setPixmap(createColoredIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/check_circle.svg",
                                           QSize(64, 64), QColor("#7c9cff")));
    iconLabel->setAlignment(Qt::AlignCenter);
    noAlertsLayout->addWidget(iconLabel);

    // Title with proper styling
    QLabel *titleLabel = new QLabel("All Clear");
    titleLabel->setObjectName("noAlertsTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    noAlertsLayout->addWidget(titleLabel);

    // Description with proper word wrapping
    QLabel *descLabel = new QLabel("There are currently no weather alerts for this location.");
    descLabel->setObjectName("noAlertsDescription");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    noAlertsLayout->addWidget(descLabel);

    // Add vertical spacer for better centering
    noAlertsLayout->addStretch(1);

    // Add to main layout
    layout()->addWidget(m_noAlertsContainer);
}

void AlertsSection::updateAlerts(const QJsonArray &alerts) {
    // Clear existing alerts properly
    while (QLayoutItem *item = m_alertsLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    // Re-add stretch
    m_alertsLayout->addStretch(1);

    // Update alert count with proper pluralization
    int alertCount = alerts.size();
    QString countText = alertCount == 1 ? "1 Active" : QString("%1 Active").arg(alertCount);
    m_alertCountLabel->setText(countText);

    // Show appropriate view
    if (alerts.isEmpty()) {
        m_noAlertsContainer->setVisible(true);
        m_scrollArea->setVisible(false);
        return;
    }

    m_noAlertsContainer->setVisible(false);
    m_scrollArea->setVisible(true);

    // Create alert cards with improved animation
    QSequentialAnimationGroup *animGroup = new QSequentialAnimationGroup(this);

    for (int i = 0; i < alerts.size(); ++i) {
        QJsonObject alertObj = alerts[i].toObject();

        // Create alert card
        AlertCard *card = new AlertCard(alertObj, m_alertsContainer);

        // Setup fade-in animation
        QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect(card);
        card->setGraphicsEffect(opacity);
        opacity->setOpacity(0.0);

        QPropertyAnimation *fadeAnim = new QPropertyAnimation(opacity, "opacity");
        fadeAnim->setDuration(200);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutQuart);

        // Add small delay between cards for staggered effect
        if (i > 0) {
            QPropertyAnimation *delayAnim = new QPropertyAnimation();
            delayAnim->setDuration(50);
            animGroup->addAnimation(delayAnim);
        }

        animGroup->addAnimation(fadeAnim);

        // Insert before the stretch item
        m_alertsLayout->insertWidget(m_alertsLayout->count() - 1, card);
    }

    // Start staggered animations
    animGroup->start(QAbstractAnimation::DeleteWhenStopped);

    // Ensure proper layout after adding cards
    m_alertsContainer->adjustSize();
}

QString AlertsSection::getSeverityStyleClass(const QString &severity) {
    QString lowerSeverity = severity.toLower();
    if (lowerSeverity == "severe" || lowerSeverity == "extreme") {
        return "severe";
    } else if (lowerSeverity == "moderate") {
        return "moderate";
    } else {
        return "minor";
    }
}

QString AlertsSection::getTimeRemainingText(qint64 endTime) {
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    qint64 remainingSecs = endTime - currentTime;

    if (remainingSecs <= 0) {
        return "Expired";
    }

    int days = remainingSecs / 86400;
    remainingSecs %= 86400;
    int hours = remainingSecs / 3600;
    remainingSecs %= 3600;
    int minutes = remainingSecs / 60;

    if (days > 0) {
        return QString("%1d %2h remaining").arg(days).arg(hours);
    } else if (hours > 0) {
        return QString("%1h %2m remaining").arg(hours).arg(minutes);
    } else if (minutes > 0) {
        return QString("%1m remaining").arg(minutes);
    } else {
        return "Less than 1m remaining";
    }
}
