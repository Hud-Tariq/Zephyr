#ifndef MAPSSECTION_H
#define MAPSSECTION_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QPixmap> // <-- Include QPixmap

// Forward declarations
class QLabel;
class QComboBox;
class QNetworkReply;

class MapsSection : public QWidget {
    Q_OBJECT
public:
    explicit MapsSection(QWidget *parent = nullptr);
    void fetchMap(double lat, double lon);

private slots:
    void onMapTypeChanged();
    void onBaseMapReplyFinished(QNetworkReply *reply);   // <-- New slot for base map
    void onOverlayReplyFinished(QNetworkReply *reply); // <-- New slot for weather overlay
    void updateMapStatus(const QString &status, const QString &message);
    void updateLayerInfo();
private:
    void combineAndDisplayMaps(); // <-- New helper function
    QLabel *m_statusIndicator;
    QLabel *m_statusLabel;
    QLabel *m_layerInfo;
    QLabel *m_legendLabel;
    QNetworkAccessManager *m_networkManager;
    QLabel *m_mapLabel;
    QComboBox *m_mapTypeCombo;
    double m_lat = 51.50;
    double m_lon = -0.12;

    // Pixmaps to store the downloaded images
    QPixmap m_baseMap;
    QPixmap m_overlayMap;
};

#endif // MAPSSECTION_H
