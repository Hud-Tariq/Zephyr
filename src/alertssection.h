#ifndef ALERTSSECTION_H
#define ALERTSSECTION_H

#include <QWidget>
#include <QJsonArray>
#include <QJsonObject>
#include <QPropertyAnimation>

// Forward declarations
class QVBoxLayout;
class QLabel;
class QPushButton;
class QScrollArea;

class AlertCard : public QWidget {
    Q_OBJECT
public:
    explicit AlertCard(const QJsonObject &alertData, QWidget *parent = nullptr);
    void setExpanded(bool expanded);

private slots:
    void toggleExpand();

private:
    bool m_expanded;
    QLabel *m_titleLabel;
    QLabel *m_timeLabel;
    QLabel *m_descriptionLabel;
    QPushButton *m_expandButton;
    QWidget *m_detailsContainer;
    QPropertyAnimation *m_expandAnimation;
    int m_expandedHeight;
};

class AlertsSection : public QWidget {
    Q_OBJECT
public:
    explicit AlertsSection(QWidget *parent = nullptr);
    void updateAlerts(const QJsonArray &alerts);

private:
    QWidget *m_noAlertsContainer;
    QVBoxLayout *m_alertsLayout;
    QScrollArea *m_scrollArea;
    QWidget *m_alertsContainer;
    QLabel *m_alertCountLabel;

    void createNoAlertsView();
    QString getSeverityStyleClass(const QString &severity);
    QString getTimeRemainingText(qint64 endTime);
};

#endif // ALERTSSECTION_H
