// mainwindow.h (updated header for animated mainwindow.cpp)
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextToSpeech>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QVBoxLayout>
#include <QTimer>
#include <QJsonObject>
#include <QGroupBox>
#include <QComboBox>
#include <QStyle>
#include <QList>
#include <QSvgRenderer>
#include <QCompleter>
#include <QColorDialog>
#include <QInputDialog>
#include <QDialog>
#include <QGraphicsEffect>
#include <QCalendarWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QtSvg>
#include <QPainter>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QGeoCoordinate>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class IWeatherClient;
class ITTSProvider;
class SettingsDialog;
class WeatherCard;
class ForecastSection;
class CitiesSection;
class MapsSection;
class ForecastsSection;
class AlertsSection;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setWeatherClient(IWeatherClient *client);
    void setTTSProvider(ITTSProvider *tts);

private slots:
    void onSearchClicked();
    void handleWeatherData(const QJsonObject &data);
    void toggleTheme();
    void showHelp();
    void updateUnits();
    void showSection(const QString &ion);
    void showSection(int index);
    void saveSettings();
    void useCurrentLocation();
    void addFavoriteCity();
    void onCitySelected(QListWidgetItem *item);
    void onCitySelectedFromFavorites(const QString &city);
    void updateCityTime();
    void onWeatherBriefingClicked();
    void onTtsStateChanged(QTextToSpeech::State state);

private:
    // UI Elements
    QLabel *statusLabel;
    QWidget *loadingOverlay;
    QPropertyAnimation *fadeAnimation;
    QStackedWidget *mainStack;
    QLineEdit *cityInput;
    QPushButton *ttsButton;
    QPushButton *themeToggle;
    SettingsDialog *settingsDialog;

    // Navigation
    QList<QPushButton*> navButtons;
    QList<QLabel*> metricLabels;

    // Services
    IWeatherClient *weatherClient;
    QTextToSpeech *speechEngine;
    ITTSProvider *ttsProvider;
    bool isDarkTheme;

    // UI Setup methods
    void setupMainUI();
    void setupSettingsUI();
    void setupAnimations();
    void applyTheme();
    void applyInitialAnimations();
    void showLoadingOverlay(bool show);
    QString generateWeatherBriefingText();

    // Widget creation methods
    QWidget* createWeatherSection();
    QWidget* createAlertsSection();

    // Animations
    void animateWidgetEntrance(QWidget *widget, int delay = 0);
    void bounceThemeToggle();

    // UI elements for animation
    QWidget *weatherCard;
    QWidget *statsContainer;
};

#endif // MAINWINDOW_H
