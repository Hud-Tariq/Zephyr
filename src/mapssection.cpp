#include "mapssection.h"
#include "apikey.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QNetworkReply>
#include <QPainter>
#include <QFrame>
// #include <QGraphicsDropShadowEffect> // This include was not used and has been removed.
#include <QPropertyAnimation>
#include <QEasingCurve>

MapsSection::MapsSection(QWidget *parent) : QWidget(parent) {
    // We need two separate managers to handle replies cleanly
    QNetworkAccessManager *baseManager = new QNetworkAccessManager(this);
    connect(baseManager, &QNetworkAccessManager::finished, this, &MapsSection::onBaseMapReplyFinished);

    QNetworkAccessManager *overlayManager = new QNetworkAccessManager(this);
    connect(overlayManager, &QNetworkAccessManager::finished, this, &MapsSection::onOverlayReplyFinished);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(24);

    // Enhanced header section with card container
    QFrame *headerCard = new QFrame;
    headerCard->setObjectName("mapHeaderCard");
    QVBoxLayout *headerCardLayout = new QVBoxLayout(headerCard);
    headerCardLayout->setContentsMargins(20, 16, 20, 16);
    headerCardLayout->setSpacing(12);

    // Title and controls in header
    QHBoxLayout *headerLayout = new QHBoxLayout;
    QLabel *title = new QLabel("Weather Maps");
    title->setObjectName("sectionTitle");

    // Add subtitle for better context
    QLabel *subtitle = new QLabel("Interactive weather overlays with real-time data");
    subtitle->setObjectName("mapSubtitle");

    QVBoxLayout *titleLayout = new QVBoxLayout;
    titleLayout->setSpacing(4);
    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    // Enhanced combo box with icon and styling
    QLabel *comboLabel = new QLabel("Layer:");
    comboLabel->setObjectName("mapComboLabel");

    m_mapTypeCombo = new QComboBox;
    m_mapTypeCombo->setObjectName("mapTypeCombo");
    m_mapTypeCombo->addItem("🌡️  Temperature", "temp_new");
    m_mapTypeCombo->addItem("🌧️  Precipitation", "precipitation_new");
    m_mapTypeCombo->addItem("💨  Wind Speed", "wind_new");
    m_mapTypeCombo->addItem("☁️  Clouds", "clouds_new");

    QHBoxLayout *comboLayout = new QHBoxLayout;
    comboLayout->setSpacing(8);
    comboLayout->addWidget(comboLabel);
    comboLayout->addWidget(m_mapTypeCombo);

    headerLayout->addLayout(comboLayout);
    headerCardLayout->addLayout(headerLayout);
    layout->addWidget(headerCard);

    // Enhanced map container with premium styling
    QFrame *mapCard = new QFrame;
    mapCard->setObjectName("mapCard");
    QVBoxLayout *mapCardLayout = new QVBoxLayout(mapCard);
    mapCardLayout->setContentsMargins(0, 0, 0, 0);

    // Map status indicator
    QFrame *statusBar = new QFrame;
    statusBar->setObjectName("mapStatusBar");
    QHBoxLayout *statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(16, 8, 16, 8);

    m_statusIndicator = new QLabel("●");
    m_statusIndicator->setObjectName("mapStatusIndicator");

    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setObjectName("mapStatusLabel");

    statusLayout->addWidget(m_statusIndicator);
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();

    // Add layer info
    m_layerInfo = new QLabel("Select a weather layer to view data");
    m_layerInfo->setObjectName("mapLayerInfo");
    statusLayout->addWidget(m_layerInfo);

    mapCardLayout->addWidget(statusBar);

    // Main map display with enhanced placeholder
    m_mapLabel = new QLabel;
    m_mapLabel->setObjectName("mapContainer");
    m_mapLabel->setAlignment(Qt::AlignCenter);
    m_mapLabel->setMinimumSize(700, 450);

    // Create enhanced placeholder content
    QFrame *placeholderFrame = new QFrame(m_mapLabel);
    placeholderFrame->setObjectName("mapPlaceholder");
    QVBoxLayout *placeholderLayout = new QVBoxLayout(placeholderFrame);
    placeholderLayout->setAlignment(Qt::AlignCenter);
    placeholderLayout->setSpacing(16);

    QLabel *placeholderIcon = new QLabel("🗺️");
    placeholderIcon->setObjectName("mapPlaceholderIcon");
    placeholderIcon->setAlignment(Qt::AlignCenter);

    QLabel *placeholderText = new QLabel("Search for a city to load interactive weather maps");
    placeholderText->setObjectName("mapPlaceholderText");
    placeholderText->setAlignment(Qt::AlignCenter);
    placeholderText->setWordWrap(true);

    QLabel *placeholderHint = new QLabel("Real-time weather data overlays on detailed base maps");
    placeholderHint->setObjectName("mapPlaceholderHint");
    placeholderHint->setAlignment(Qt::AlignCenter);

    placeholderLayout->addWidget(placeholderIcon);
    placeholderLayout->addWidget(placeholderText);
    placeholderLayout->addWidget(placeholderHint);

    // Position placeholder in center of map label
    placeholderFrame->resize(400, 200);
    placeholderFrame->move((m_mapLabel->width() - 400) / 2, (m_mapLabel->height() - 200) / 2);

    mapCardLayout->addWidget(m_mapLabel, 1);
    layout->addWidget(mapCard, 1);

    // Map legend/info panel
    QFrame *legendCard = new QFrame;
    legendCard->setObjectName("mapLegendCard");
    QHBoxLayout *legendLayout = new QHBoxLayout(legendCard);
    legendLayout->setContentsMargins(20, 12, 20, 12);

    m_legendLabel = new QLabel("💡 Tip: Weather overlays are semi-transparent to show underlying geography");
    m_legendLabel->setObjectName("mapLegendLabel");

    legendLayout->addWidget(m_legendLabel);
    legendLayout->addStretch();
    layout->addWidget(legendCard);

    connect(m_mapTypeCombo, &QComboBox::currentIndexChanged, this, &MapsSection::onMapTypeChanged);
}

void MapsSection::fetchMap(double lat, double lon) {
    m_lat = lat;
    m_lon = lon;

    // Update status
    updateMapStatus("preparing", "Preparing map data...");
    updateLayerInfo();

    onMapTypeChanged();
}

void MapsSection::onMapTypeChanged() {
    // Hide placeholder and show loading state
    QFrame *placeholder = m_mapLabel->findChild<QFrame*>("mapPlaceholder");
    if (placeholder) {
        placeholder->hide();
    }

    updateMapStatus("loading", "Loading map layers...");

    // Clear old images
    m_baseMap = QPixmap();
    m_overlayMap = QPixmap();

    int zoom = 8;
    int n = 1 << zoom;
    int xtile = static_cast<int>(floor((m_lon + 180.0) / 360.0 * n));
    int ytile = static_cast<int>(floor((1.0 - asinh(tan(m_lat * M_PI / 180.0)) / M_PI) / 2.0 * n));

    // --- Request 1: The Base Map (from a free provider like CARTO) ---
    QNetworkAccessManager *baseManager = new QNetworkAccessManager(this);
    connect(baseManager, &QNetworkAccessManager::finished, this, &MapsSection::onBaseMapReplyFinished);
    QUrl baseUrl(QString("https://a.basemaps.cartocdn.com/rastertiles/voyager/%1/%2/%3.png").arg(zoom).arg(xtile).arg(ytile));
    baseManager->get(QNetworkRequest(baseUrl));

    // --- Request 2: The Weather Overlay (from OpenWeatherMap) ---
    QNetworkAccessManager *overlayManager = new QNetworkAccessManager(this);
    connect(overlayManager, &QNetworkAccessManager::finished, this, &MapsSection::onOverlayReplyFinished);
    QString layer = m_mapTypeCombo->currentData().toString();
    QUrl overlayUrl(QString("https://tile.openweathermap.org/map/%1/%2/%3/%4.png?appid=%5")
                        .arg(layer).arg(zoom).arg(xtile).arg(ytile).arg(API_KEY));
    overlayManager->get(QNetworkRequest(overlayUrl));

    updateLayerInfo();
}

void MapsSection::onBaseMapReplyFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        m_baseMap.loadFromData(reply->readAll());
        updateMapStatus("loading", "Base map loaded, loading weather data...");
        combineAndDisplayMaps();
    } else {
        updateMapStatus("error", "Failed to load base map");
    }
    reply->deleteLater();
    reply->manager()->deleteLater();
}

void MapsSection::onOverlayReplyFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        m_overlayMap.loadFromData(reply->readAll());
        updateMapStatus("loading", "Weather overlay loaded, combining layers...");
        combineAndDisplayMaps();
    } else {
        updateMapStatus("error", "Failed to load weather overlay");
    }
    reply->deleteLater();
    reply->manager()->deleteLater();
}

void MapsSection::combineAndDisplayMaps() {
    // Wait until both pixmaps have been loaded
    if (m_baseMap.isNull() || m_overlayMap.isNull()) {
        return;
    }

    // Create a new pixmap to draw on
    QPixmap finalMap = m_baseMap;

    // Use QPainter to draw the weather overlay on top of the base map
    QPainter painter(&finalMap);
    painter.setOpacity(0.7); // Make the weather layer semi-transparent
    painter.drawPixmap(0, 0, m_overlayMap);
    painter.end();

    // Display the final, combined image with smooth scaling
    m_mapLabel->setPixmap(finalMap.scaled(m_mapLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    updateMapStatus("ready", "Map loaded successfully");
}

void MapsSection::updateMapStatus(const QString &status, const QString &message) {
    m_statusLabel->setText(message);

    if (status == "loading") {
        m_statusIndicator->setObjectName("mapStatusIndicatorLoading");
    } else if (status == "ready") {
        m_statusIndicator->setObjectName("mapStatusIndicatorReady");
    } else if (status == "error") {
        m_statusIndicator->setObjectName("mapStatusIndicatorError");
    } else {
        m_statusIndicator->setObjectName("mapStatusIndicator");
    }

    // Force style update
    m_statusIndicator->style()->unpolish(m_statusIndicator);
    m_statusIndicator->style()->polish(m_statusIndicator);
}

void MapsSection::updateLayerInfo() {
    QString currentLayer = m_mapTypeCombo->currentText();
    QString info;

    if (currentLayer.contains("Temperature")) {
        info = "Showing temperature distribution across the region";
    } else if (currentLayer.contains("Precipitation")) {
        info = "Displaying precipitation levels and intensity";
    } else if (currentLayer.contains("Wind")) {
        info = "Visualizing wind speed patterns";
    } else if (currentLayer.contains("Clouds")) {
        info = "Cloud coverage and density overlay";
    }

    m_layerInfo->setText(info);
}
