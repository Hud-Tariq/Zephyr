#ifndef CITIESSECTION_H
#define CITIESSECTION_H

#include <QWidget>

// Forward declarations
class QListWidget;
class QLineEdit;

class CitiesSection : public QWidget {
    Q_OBJECT

public:
    explicit CitiesSection(QWidget *parent = nullptr);
    void addCityToList(const QString &city); // New public function

private slots:
    void filterCities(const QString &text); // New slot for the search bar

signals:
    void addCityClicked();
    void citySelected(const QString &city);

private:
    QListWidget *m_citiesList; // Pointer to the list widget
    QLineEdit *m_citySearch;   // Pointer to the search input
};

#endif // CITIESSECTION_H
