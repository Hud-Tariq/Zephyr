#include "SettingsManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QStandardPaths>

void SettingsManager::saveSettings(const QString& theme, const QString& lastCity) {
    QJsonObject settings;
    settings["theme"] = theme;
    settings["lastCity"] = lastCity;

    QJsonDocument doc(settings);
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(path + "/settings.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

QJsonObject SettingsManager::loadSettings() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/settings.json";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonObject(); // Return empty if file doesn't exist
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    return doc.object();
}
