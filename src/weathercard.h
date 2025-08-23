#ifndef WEATHERCARD_H
#define WEATHERCARD_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

class WeatherCard : public QWidget {
    Q_OBJECT

public:
    explicit WeatherCard(QWidget *parent = nullptr);

    // --- FIX ---
    // The public member pointers have been removed. We will access the labels
    // safely from MainWindow using their object names. Keeping uninitialized
    // public pointers is a source of bugs and confusion.
    // We only keep the ones that are DIRECTLY manipulated in the constructor.
    QLabel *tempLabel;
    QLabel *conditionLabel;
    QLabel *conditionIcon;
    QLabel *sunriseLabel;
    QLabel *sunsetLabel;
};

#endif // WEATHERCARD_H
