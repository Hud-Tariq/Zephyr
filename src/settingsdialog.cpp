#include "settingsdialog.h"
#include <QLabel>
SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Settings");
    setMinimumWidth(600);
    setObjectName("settingsDialog");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(20);

    // Tab widget for settings categories
    QTabWidget *settingsTabs = new QTabWidget;
    settingsTabs->setObjectName("settingsTabs");

    setupUnitsTab(settingsTabs);
    setupAppearanceTab(settingsTabs);
    setupNotificationsTab(settingsTabs);

    layout->addWidget(settingsTabs);

    // Dialog buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();

    QPushButton *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("secondaryButton");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *saveBtn = new QPushButton("Save Settings");
    saveBtn->setObjectName("primaryButton");
    connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);

    layout->addLayout(buttonLayout);
}

void SettingsDialog::setupUnitsTab(QTabWidget *settingsTabs) {
    QWidget *unitsTab = new QWidget;
    QVBoxLayout *unitsLayout = new QVBoxLayout(unitsTab);
    unitsLayout->setSpacing(20);

    QLabel *unitsTitle = new QLabel("Measurement Units");
    unitsTitle->setObjectName("settingsTitle");
    unitsLayout->addWidget(unitsTitle);

    // Initialize combo boxes
    QWidget *unitsGrid = new QWidget;
    QFormLayout *unitsForm = new QFormLayout(unitsGrid);
    unitsForm->setSpacing(15);
    unitsForm->setLabelAlignment(Qt::AlignRight);

    tempUnitCombo = new QComboBox();
    tempUnitCombo->setObjectName("settingsCombo");
    tempUnitCombo->addItems({"Celsius", "Fahrenheit"});

    windUnitCombo = new QComboBox();
    windUnitCombo->setObjectName("settingsCombo");
    windUnitCombo->addItems({"km/h", "m/s", "mph", "knots"});

    pressureUnitCombo = new QComboBox();
    pressureUnitCombo->setObjectName("settingsCombo");
    pressureUnitCombo->addItems({"hPa", "inHg", "mmHg", "kPa"});

    distanceUnitCombo = new QComboBox();
    distanceUnitCombo->setObjectName("settingsCombo");
    distanceUnitCombo->addItems({"Kilometers", "Miles"});

    precipitationUnitCombo = new QComboBox();
    precipitationUnitCombo->setObjectName("settingsCombo");
    precipitationUnitCombo->addItems({"Millimeters", "Inches"});

    timeFormatCombo = new QComboBox();
    timeFormatCombo->setObjectName("settingsCombo");
    timeFormatCombo->addItems({"24-hour", "12-hour"});

    unitsForm->addRow("Temperature:", tempUnitCombo);
    unitsForm->addRow("Wind Speed:", windUnitCombo);
    unitsForm->addRow("Pressure:", pressureUnitCombo);
    unitsForm->addRow("Distance:", distanceUnitCombo);
    unitsForm->addRow("Precipitation:", precipitationUnitCombo);
    unitsForm->addRow("Time Format:", timeFormatCombo);

    unitsLayout->addWidget(unitsGrid);
    unitsLayout->addStretch();

    settingsTabs->addTab(unitsTab, "Units");
}

void SettingsDialog::setupAppearanceTab(QTabWidget *settingsTabs) {
    QWidget *appearanceTab = new QWidget;
    QVBoxLayout *appearanceLayout = new QVBoxLayout(appearanceTab);
    appearanceLayout->setSpacing(20);

    QLabel *appearanceTitle = new QLabel("Appearance");
    appearanceTitle->setObjectName("settingsTitle");
    appearanceLayout->addWidget(appearanceTitle);

    QWidget *appearanceGrid = new QWidget;
    QFormLayout *appearanceForm = new QFormLayout(appearanceGrid);
    appearanceForm->setSpacing(15);
    appearanceForm->setLabelAlignment(Qt::AlignRight);

    // Theme selection
    QComboBox *themeCombo = new QComboBox();
    themeCombo->setObjectName("settingsCombo");
    themeCombo->addItems({"Dark", "Light", "System Default"});

    // Home screen layout
    QComboBox *layoutCombo = new QComboBox();
    layoutCombo->setObjectName("settingsCombo");
    layoutCombo->addItems({"Standard", "Compact", "Expanded"});

    // Font size
    QComboBox *fontSizeCombo = new QComboBox();
    fontSizeCombo->setObjectName("settingsCombo");
    fontSizeCombo->addItems({"Small", "Medium", "Large"});
    fontSizeCombo->setCurrentIndex(1);

    // Custom accent color
    QPushButton *accentColorBtn = new QPushButton;
    accentColorBtn->setObjectName("colorButton");
    accentColorBtn->setStyleSheet("background-color: #3498db;");
    accentColorBtn->setFixedSize(40, 40);
    connect(accentColorBtn, &QPushButton::clicked, [accentColorBtn]() {
        QColor color = QColorDialog::getColor(Qt::blue, nullptr, "Select Accent Color");
        if (color.isValid()) {
            accentColorBtn->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        }
    });

    appearanceForm->addRow("Theme:", themeCombo);
    appearanceForm->addRow("Layout:", layoutCombo);
    appearanceForm->addRow("Font Size:", fontSizeCombo);
    appearanceForm->addRow("Accent Color:", accentColorBtn);

    appearanceLayout->addWidget(appearanceGrid);
    appearanceLayout->addStretch();

    settingsTabs->addTab(appearanceTab, "Appearance");
}

void SettingsDialog::setupNotificationsTab(QTabWidget *settingsTabs) {
    QWidget *notificationsTab = new QWidget;
    QVBoxLayout *notificationsLayout = new QVBoxLayout(notificationsTab);
    notificationsLayout->setSpacing(20);

    QLabel *notificationsTitle = new QLabel("Notifications");
    notificationsTitle->setObjectName("settingsTitle");
    notificationsLayout->addWidget(notificationsTitle);

    QWidget *notificationsGrid = new QWidget;
    QFormLayout *notificationsForm = new QFormLayout(notificationsGrid);
    notificationsForm->setSpacing(15);
    notificationsForm->setLabelAlignment(Qt::AlignRight);

    QCheckBox *enableAlerts = new QCheckBox("Enable weather alerts");
    enableAlerts->setChecked(true);

    QCheckBox *enableSound = new QCheckBox("Enable alert sounds");
    enableSound->setChecked(true);

    QCheckBox *enableVibration = new QCheckBox("Enable vibration");

    QComboBox *alertLevelCombo = new QComboBox();
    alertLevelCombo->addItems({"All alerts", "Moderate and severe", "Severe only"});

    notificationsForm->addRow(enableAlerts);
    notificationsForm->addRow(enableSound);
    notificationsForm->addRow(enableVibration);
    notificationsForm->addRow("Alert Level:", alertLevelCombo);

    notificationsLayout->addWidget(notificationsGrid);
    notificationsLayout->addStretch();

    settingsTabs->addTab(notificationsTab, "Notifications");
}
