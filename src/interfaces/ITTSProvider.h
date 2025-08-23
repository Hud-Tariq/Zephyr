#pragma once
#include <QObject>
#include <QString>

class ITTSProvider : public QObject {
    Q_OBJECT
public:
    virtual void speak(const QString &text) = 0;
};
