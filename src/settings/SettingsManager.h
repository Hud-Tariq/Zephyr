#pragma once

#include <QString>
#include <QJsonObject>

class SettingsManager {
public:
    static void saveSettings(const QString& theme, const QString& lastCity);
    static QJsonObject loadSettings();
};
