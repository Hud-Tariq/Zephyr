#include "citiessection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QIcon>
#include <QFrame>

CitiesSection::CitiesSection(QWidget *parent) : QWidget(parent) {
    setObjectName("citiesSection");
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(20);

    // Header section with background
    QFrame *headerFrame = new QFrame;
    headerFrame->setObjectName("citiesHeader");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(20, 16, 20, 16);
    headerLayout->setSpacing(12);

    QLabel *title = new QLabel("Favorite Cities");
    title->setObjectName("sectionTitle");
    headerLayout->addWidget(title);
    headerLayout->addStretch();

    QPushButton *addBtn = new QPushButton("Add City");
    addBtn->setObjectName("primaryButton");
    addBtn->setIcon(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/add.svg"));
    connect(addBtn, &QPushButton::clicked, this, &CitiesSection::addCityClicked);
    headerLayout->addWidget(addBtn);

    layout->addWidget(headerFrame);

    // Search container
    QFrame *searchFrame = new QFrame;
    searchFrame->setObjectName("searchContainer");
    QHBoxLayout *searchLayout = new QHBoxLayout(searchFrame);
    searchLayout->setContentsMargins(0, 0, 0, 0);

    m_citySearch = new QLineEdit;
    m_citySearch->setPlaceholderText("Search favorite cities...");
    m_citySearch->setObjectName("searchInput");
    m_citySearch->setClearButtonEnabled(true);
    connect(m_citySearch, &QLineEdit::textChanged, this, &CitiesSection::filterCities);
    searchLayout->addWidget(m_citySearch);

    layout->addWidget(searchFrame);

    // Cities list container
    QFrame *listFrame = new QFrame;
    listFrame->setObjectName("citiesListContainer");
    QVBoxLayout *listLayout = new QVBoxLayout(listFrame);
    listLayout->setContentsMargins(0, 0, 0, 0);

    m_citiesList = new QListWidget;
    m_citiesList->setObjectName("citiesList");
    m_citiesList->setIconSize(QSize(24, 24));
    m_citiesList->setSpacing(2);
    m_citiesList->setAlternatingRowColors(true);

    // When an item is clicked, emit the citySelected signal
    connect(m_citiesList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        emit citySelected(item->data(Qt::UserRole).toString());
    });

    listLayout->addWidget(m_citiesList);
    layout->addWidget(listFrame, 1);
}

// Public function for MainWindow to add a city to our list
void CitiesSection::addCityToList(const QString &city) {
    // Prevent duplicates
    if (!m_citiesList->findItems(city, Qt::MatchExactly).isEmpty()) {
        return;
    }

    QListWidgetItem *item = new QListWidgetItem(city, m_citiesList);
    item->setIcon(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/city.svg"));
    item->setData(Qt::UserRole, city);
}

// Slot that filters the list based on the search text
void CitiesSection::filterCities(const QString &text) {
    for (int i = 0; i < m_citiesList->count(); ++i) {
        QListWidgetItem *item = m_citiesList->item(i);
        item->setHidden(!item->text().contains(text, Qt::CaseInsensitive));
    }
}
