#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>
#include <QLabel>
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    QComboBox *tempUnitCombo;
    QComboBox *windUnitCombo;
    QComboBox *pressureUnitCombo;
    QComboBox *distanceUnitCombo;
    QComboBox *precipitationUnitCombo;
    QComboBox *timeFormatCombo;

private:
    void setupUnitsTab(QTabWidget *settingsTabs);
    void setupAppearanceTab(QTabWidget *settingsTabs);
    void setupNotificationsTab(QTabWidget *settingsTabs);
};

#endif // SETTINGSDIALOG_H
