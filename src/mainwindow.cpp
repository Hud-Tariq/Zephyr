#include "mainwindow.h"
#include "weathercard.h"
#include "forecastsection.h"
#include "citiessection.h"
#include "mapssection.h"
#include "forecastssection.h"
#include "alertssection.h"
#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMovie>
#include <QListWidget>
#include <QScrollArea>
#include <QSpacerItem>
#include <QTimer>
#include <QJsonObject>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QEasingCurve>
#include <QCalendarWidget>
#include <QSlider>
#include <QCheckBox>
#include <QPainter>
#include <QSvgRenderer>
#include <QTabWidget>
#include <QChart>
#include <QLineSeries>
#include <QCategoryAxis>
#include <QValueAxis>
#include <QStyleOptionSlider>
#include <QAbstractItemView>
#include <QToolButton>
#include <QFontDatabase>
#include <QGradient>
#include <QRadialGradient>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QInputDialog>
#include <QtSVG>
#include "settings/SettingsManager.h"
#include "apikey.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTextToSpeech>
#include "IWeatherClient.h"
#include "ITTSProvider.h"
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QEasingCurve>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), loadingOverlay(nullptr), fadeAnimation(nullptr), ttsButton(nullptr),
    weatherClient(nullptr), ttsProvider(nullptr)
{
    setWindowTitle("Zephyr Weather");
    setMinimumSize(1200, 800);

    QFontDatabase::addApplicationFont("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/fonts/Montserrat-Regular.ttf");
    QFontDatabase::addApplicationFont("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/fonts/Montserrat-Bold.ttf");
    QFontDatabase::addApplicationFont("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/fonts/Montserrat-Light.ttf");

    QJsonObject settings = SettingsManager::loadSettings();
    isDarkTheme = settings["theme"].toString("dark") == "dark";
    QString lastCity = settings["lastCity"].toString("New York");

    setupMainUI();
    setupSettingsUI();
    setupAnimations();
    applyTheme();

    // Initialize Text-to-Speech Engine
    speechEngine = new QTextToSpeech(this);
    connect(speechEngine, &QTextToSpeech::stateChanged, this, &MainWindow::onTtsStateChanged);

    // Initialize weather data with last saved city
    if (!lastCity.isEmpty()) {
        cityInput->setText(lastCity);
        QTimer::singleShot(500, this, &MainWindow::onSearchClicked);
    }
}


void MainWindow::animateWidgetEntrance(QWidget *widget, int delay)
{
    if (!widget) return;
    QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(opacity);
    opacity->setOpacity(0);

    QPropertyAnimation *fadeIn = new QPropertyAnimation(opacity, "opacity", this);
    fadeIn->setStartValue(0);
    fadeIn->setEndValue(1);
    fadeIn->setDuration(500);
    fadeIn->setEasingCurve(QEasingCurve::InOutQuad);

    QRect endGeom = widget->geometry();
    QRect startGeom = endGeom.adjusted(0, 50, 0, 50);

    QPropertyAnimation *slideIn = new QPropertyAnimation(widget, "geometry", this);
    slideIn->setStartValue(startGeom);
    slideIn->setEndValue(endGeom);
    slideIn->setDuration(500);
    slideIn->setEasingCurve(QEasingCurve::OutCubic);


    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(fadeIn);
    group->addAnimation(slideIn);

    if (delay > 0) {
        QTimer::singleShot(delay, group, SLOT(start()));
    } else {
        group->start();
    }
}


void MainWindow::setupMainUI()
{
    QWidget *mainWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header with app logo and title
    QWidget *header = new QWidget;
    header->setObjectName("appHeader");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);


    QLabel *logoLabel = new QLabel;
    QSvgRenderer renderer(QString("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/zephyr_logo.svg"));
    QPixmap logo(42, 42);
    logo.fill(Qt::transparent);
    QPainter painter(&logo);
    renderer.render(&painter);
    logoLabel->setPixmap(logo);

    QLabel *titleLabel = new QLabel("ZEPHYR");
    titleLabel->setObjectName("appTitle");

    headerLayout->addWidget(logoLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    // Theme toggle button
    themeToggle = new QPushButton;
    themeToggle->setObjectName("iconButton");
    themeToggle->setFixedSize(36, 36);
    connect(themeToggle, &QPushButton::clicked, this, [this]() {
        bounceThemeToggle();
        toggleTheme();
    });

    headerLayout->addWidget(themeToggle);

    mainLayout->addWidget(header);

    // Navigation and content container
    QWidget *contentContainer = new QWidget;
    contentContainer->setObjectName("contentContainer");
    QHBoxLayout *containerLayout = new QHBoxLayout(contentContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Side Navigation
    QWidget *sideNav = new QWidget;
    sideNav->setObjectName("sideNav");
    sideNav->setFixedWidth(220);
    QVBoxLayout *navLayout = new QVBoxLayout(sideNav);
    navLayout->setContentsMargins(20, 30, 20, 20);
    navLayout->setSpacing(10);

    QLabel *navTitle = new QLabel("Dashboard");
    navTitle->setObjectName("navSectionTitle");
    navLayout->addWidget(navTitle);

    QStringList navItems = {"Weather", "Cities", "Maps", "Forecasts", "Alerts"};
    QStringList navIcons = {
        "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/weather.svg",
        "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/cities.svg",
        "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/maps.svg",
        "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/forecast.svg",
        "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/alerts.svg"
    };

    for (int i = 0; i < navItems.size(); i++) {
        QPushButton *btn = new QPushButton(navItems[i]);
        btn->setObjectName("navButton");
        btn->setCheckable(true);
        btn->setProperty("navIndex", i);

        // Load SVG and render it to a pixmap
        QSvgRenderer svgRenderer(navIcons[i]);
        QPixmap pixmap(22, 22);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        svgRenderer.render(&painter);
        painter.end();

        btn->setIcon(QIcon(pixmap));
        btn->setIconSize(QSize(22, 22));

        navLayout->addWidget(btn);
        navButtons.append(btn);

        connect(btn, &QPushButton::clicked, this, [this, i]() { showSection(i); });
    }
    navLayout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QLabel *settingsTitle = new QLabel("Preferences");
    settingsTitle->setObjectName("navSectionTitle");
    navLayout->addWidget(settingsTitle);

    QPushButton *settingsBtn = new QPushButton("Settings");
    settingsBtn->setObjectName("navButton");
    settingsBtn->setIcon(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/settings.svg"));
    settingsBtn->setIconSize(QSize(22, 22));
    connect(settingsBtn, &QPushButton::clicked, this, [this](){ showSection("Settings"); });
    navLayout->addWidget(settingsBtn);

    QPushButton *helpBtn = new QPushButton("Help");
    helpBtn->setObjectName("navButton");
    helpBtn->setIcon(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/help.svg"));
    helpBtn->setIconSize(QSize(22, 22));
    connect(helpBtn, &QPushButton::clicked, this, &MainWindow::showHelp);
    navLayout->addWidget(helpBtn);

    // Content Stack
    QWidget *contentWrapper = new QWidget;
    contentWrapper->setObjectName("contentWrapper");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWrapper);
    contentLayout->setContentsMargins(30, 30, 30, 30);

    mainStack = new QStackedWidget;
    mainStack->setObjectName("mainStack");

    // Add all sections
    mainStack->addWidget(createWeatherSection());
    CitiesSection *citiesSection = new CitiesSection;
    mainStack->addWidget(citiesSection);

    // Connect the signals from CitiesSection to slots in MainWindow
    connect(citiesSection, &CitiesSection::citySelected, this, &MainWindow::onCitySelectedFromFavorites);
    connect(citiesSection, &CitiesSection::addCityClicked, this, &MainWindow::addFavoriteCity);


    citiesSection->addCityToList("New York");
    citiesSection->addCityToList("London");
    citiesSection->addCityToList("Tokyo");
    mainStack->addWidget(new MapsSection);
    mainStack->addWidget(new ForecastsSection);
    mainStack->addWidget(createAlertsSection());

    contentLayout->addWidget(mainStack);

    containerLayout->addWidget(sideNav);
    containerLayout->addWidget(contentWrapper, 1);

    mainLayout->addWidget(contentContainer, 1);

    // Status bar
    QWidget *statusBar = new QWidget;
    statusBar->setObjectName("statusBar");
    QHBoxLayout *statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(20, 8, 20, 8);

    statusLabel = new QLabel("Ready");
    statusLayout->addWidget(statusLabel);

    QLabel *versionLabel = new QLabel("Zephyr Dev");
    versionLabel->setAlignment(Qt::AlignRight);
    statusLayout->addWidget(versionLabel);

    mainLayout->addWidget(statusBar);

    setCentralWidget(mainWidget);

    // Set the first tab as active by default
    if (!navButtons.isEmpty()) {
        navButtons[0]->setChecked(true);
    }
}

static QPixmap getWeatherIcon(const QString &mainCondition, const QString &description, bool isNight, int size = 32) {
    QMap<QString, QString> iconMap;

    iconMap["clear sky"] = isNight ? "clear-night.svg" : "clear-day.svg";
    iconMap["few clouds"] = isNight ? "partly-cloudy-night.svg" : "partly-cloudy-day.svg";
    iconMap["scattered clouds"] = isNight ? "partly-cloudy-night.svg" : "partly-cloudy-day.svg";
    iconMap["broken clouds"] = "cloudy.svg";
    iconMap["overcast clouds"] = "overcast.svg";
    iconMap["shower rain"] = "rain.svg";
    iconMap["light rain"] = "drizzle.svg";
    iconMap["moderate rain"] = "rain.svg";

    iconMap["clouds"] = "cloudy.svg";
    iconMap["rain"] = "rain.svg";
    iconMap["drizzle"] = "drizzle.svg";
    iconMap["snow"] = "snow.svg";
    iconMap["thunderstorm"] = "thunderstorms.svg";
    iconMap["fog"] = "fog.svg";
    iconMap["mist"] = "fog.svg";
    iconMap["haze"] = "haze.svg";
    iconMap["clear"] = isNight ? "clear-night.svg" : "clear-day.svg";


    QString lowerDesc = description.toLower();
    QString iconFile = iconMap.value(lowerDesc);

    if (iconFile.isEmpty()) {
        iconFile = iconMap.value(mainCondition.toLower());
    }

    if (iconFile.isEmpty()) {
        iconFile = "partly-cloudy-day.svg";
    }

    QString iconPath = QString("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/weather/%1").arg(iconFile);
    QIcon icon(iconPath);

    if (icon.isNull()) {
        QPixmap fallback(size, size);
        fallback.fill(Qt::transparent);
        QPainter painter(&fallback);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor("#7c9cff"));
        painter.setPen(Qt::NoPen);
        painter.drawRect(fallback.rect().adjusted(2, 2, -2, -2));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", size / 3, QFont::Bold));
        painter.drawText(fallback.rect(), Qt::AlignCenter, mainCondition.left(1).toUpper());
        return fallback;
    }

    return icon.pixmap(size, size);
}


QWidget* MainWindow::createWeatherSection()
{
    QWidget *weatherWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(weatherWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    QWidget *searchContainer = new QWidget;
    searchContainer->setObjectName("searchContainer");
    QHBoxLayout *searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(20, 15, 20, 15);

    QLabel *searchIcon = new QLabel;
    searchIcon->setPixmap(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/search.svg").pixmap(24, 24));
    searchIcon->setStyleSheet("background: transparent; border: none;");
    searchLayout->addWidget(searchIcon);

    cityInput = new QLineEdit;
    cityInput->setPlaceholderText("Search for a city...");
    cityInput->setObjectName("searchInput");
    cityInput->setClearButtonEnabled(true);

    //autocomplete
    QCompleter *completer = new QCompleter(QStringList{
        "Abidjan", "Abu Dhabi", "Abuja", "Accra", "Addis Ababa", "Ahmedabad", "Aleppo",
        "Alexandria", "Algiers", "Almaty", "Amman", "Amsterdam", "Anchorage", "Andorra la Vella",
        "Ankara", "Antananarivo", "Apia", "Arnold", "Ashgabat", "Asmara", "Asuncion", "Athens",
        "Auckland", "Avarua", "Baghdad", "Baku", "Bamako", "Banda Aceh", "Bandar Seri Begawan",
        "Bandung", "Bangkok", "Bangui", "Banjul", "Barcelona", "Barranquilla", "Basrah",
        "Basse-Terre", "Basseterre", "Beijing", "Beirut", "Bekasi", "Belem", "Belgrade",
        "Belmopan", "Belo Horizonte", "Bengaluru", "Berlin", "Bern", "Bishkek", "Bissau",
        "Bogota", "Brasilia", "Bratislava", "Brazzaville", "Bridgetown", "Brisbane", "Brussels",
        "Bucharest", "Budapest", "Buenos Aires", "Bujumbura", "Bursa", "Busan", "Cairo",
        "Cali", "Caloocan", "Camayenne", "Canberra", "Cape Town", "Caracas", "Casablanca",
        "Castries", "Cayenne", "Charlotte Amalie", "Chengdu", "Chennai", "Chicago", "Chisinau",
        "Chittagong","Pakpattan","Shanghai", "Chongqing", "Colombo", "Conakry", "Copenhagen", "Cordoba", "Curitiba",
        "Daegu", "Daejeon", "Dakar", "Dallas", "Damascus", "Dar es Salaam", "Delhi", "Denver",
        "Dhaka", "Dili", "Djibouti", "Dodoma", "Doha", "Dongguan", "Douala", "Douglas", "Dubai",
        "Dublin", "Durban", "Dushanbe", "Faisalabad", "Fort-de-France", "Fortaleza", "Freetown",
        "Fukuoka", "Funafuti", "Gaborone", "George Town", "Georgetown", "Gibraltar", "Gitega",
        "Giza", "Guadalajara", "Guangzhou", "Guatemala City", "Guayaquil", "Gujranwala",
        "Gustavia", "Gwangju", "Hamburg", "Hanoi", "Harare", "Havana", "Helsinki",
        "Ho Chi Minh City", "Hong Kong", "Honiara", "Honolulu", "Houston", "Hyderabad",
        "Ibadan", "Incheon", "Isfahan", "Islamabad", "Istanbul", "Izmir", "Jaipur", "Jakarta",
        "Jeddah", "Jerusalem", "Johannesburg", "Juarez", "Juba", "Kabul", "Kaduna", "Kampala",
        "Kano", "Kanpur", "Kaohsiung", "Karachi", "Karaj", "Kathmandu", "Kawasaki", "Kharkiv",
        "Khartoum", "Khulna", "Kigali", "Kingsburg", "Kingston", "Kingstown", "Kinshasa", "Kobe",
        "Kolkata", "Kota Bharu", "Kowloon", "Kuala Lumpur", "Kumasi", "Kuwait", "Kyiv", "Kyoto",
        "La Paz", "Lagos", "Lahore", "Libreville", "Lilongwe", "Lima", "Lisbon", "Ljubljana",
        "Lome", "London", "Los Angeles", "Luanda", "Lubumbashi", "Lusaka", "Luxembourg",
        "Macau", "Madrid", "Majuro", "Makassar", "Malabo", "Male", "Mamoudzou", "Managua",
        "Manama", "Manaus", "Manila", "Maputo", "Maracaibo", "Maracay", "Mariehamn", "Marigot",
        "Maseru", "Mashhad", "Mbabane", "Mecca", "Medan", "Medellin", "Medina", "Melbourne",
        "Mexico City", "Miami", "Minsk", "Mogadishu", "Monaco", "Monrovia", "Montevideo",
        "Montreal", "Moroni", "Moscow", "Mosul", "Multan", "Mumbai", "Muscat", "N'Djamena",
        "Nagoya", "Nairobi", "Nanchong", "Nanjing", "Nassau", "Nay Pyi Taw", "New York", "Niamey",
        "Nicosia", "Nouakchott", "Noumea", "Novosibirsk", "Nuku'alofa", "Nur-Sultan", "Nuuk",
        "Oranjestad", "Osaka", "Oslo", "Ottawa", "Ouagadougou", "Pago Pago", "Palembang",
        "Palo Alto", "Panama", "Papeete", "Paramaribo", "Paris", "Perth", "Philadelphia",
        "Phnom Penh", "Phoenix", "Podgorica", "Port Louis", "Port Moresby", "Port of Spain",
        "Port-Vila", "Port-au-Prince", "Porto Alegre", "Porto-Novo", "Prague", "Praia",
        "Pretoria", "Pristina", "Puebla", "Pune", "Pyongyang", "Quezon City", "Quito",
        "Rabat", "Rawalpindi", "Recife", "Reykjavik", "Riga", "Rio de Janeiro", "Riyadh",
        "Road Town", "Rome", "Roseau", "Saint George's", "Saint Helier", "Saint John's",
        "Saint Peter Port", "Saint Petersburg", "Saint-Denis", "Saint-Pierre", "Saipan",
        "Salvador", "San Antonio", "San Diego", "San Francisco", "San Jose", "San Juan",
        "San Marino", "San Salvador", "Sanaa", "Santa Cruz de la Sierra", "Santiago",
        "Santo Domingo", "Sao Paulo", "Sao Tome", "Sapporo", "Sarajevo", "Seattle", "Semarang",
        "Seoul", "Shanghai", "Sharjah", "Shenzhen", "Singapore", "Skopje", "Sofia", "South Tarawa",
        "Soweto", "Stockholm", "Sucre", "Surabaya", "Surat", "Suva", "Sydney", "Tabriz",
        "Taipei", "Tallinn", "Tangerang", "Tarawa", "Tashkent", "Tbilisi", "Tegucigalpa",
        "higan", "Tehran", "Tel Aviv", "Thimphu", "Tianjin", "Tijuana", "Tirana", "Tokyo",
        "Toronto", "Torshavn", "Tripoli", "Tunis", "Ulan Bator", "Vaduz", "Valencia",
        "Valletta", "Vancouver", "Victoria", "Vienna", "Vientiane", "Vilnius", "Warsaw","Wah",
        "Washington", "Wellington", "Willemstad", "Windhoek", "Wuhan", "Xi'an", "Yamoussoukro",
        "Yangon", "Yaounde", "Yekaterinburg", "Yerevan", "Yokohama", "Zagreb"
    });
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    cityInput->setCompleter(completer);

    searchLayout->addWidget(cityInput, 1);

    QPushButton *searchBtn = new QPushButton("Search");
    searchBtn->setObjectName("primaryButton");
    connect(searchBtn, &QPushButton::clicked, this, &MainWindow::onSearchClicked);
    searchLayout->addWidget(searchBtn);

    QPushButton *locationBtn = new QPushButton;
    locationBtn->setIcon(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/location.svg"));
    locationBtn->setIconSize(QSize(20, 20));
    locationBtn->setToolTip("Use current location");
    locationBtn->setObjectName("iconButton");
    locationBtn->setFixedSize(42, 42);
    connect(locationBtn, &QPushButton::clicked, this, &MainWindow::useCurrentLocation);
    searchLayout->addWidget(locationBtn);

    layout->addWidget(searchContainer);

    // Main content with scroll area for responsiveness
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *scrollContent = new QWidget;
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(20);

    // City header with time
    QWidget *cityHeader = new QWidget;
    cityHeader->setObjectName("cityHeaderWidget");
    QHBoxLayout *cityHeaderLayout = new QHBoxLayout(cityHeader);
    cityHeaderLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *cityNameLabel = new QLabel("--");
    cityNameLabel->setObjectName("cityNameLabel");

    QLabel *cityTimeLabel = new QLabel("");
    cityTimeLabel->setObjectName("cityTimeLabel");

    cityHeaderLayout->addWidget(cityNameLabel);
    cityHeaderLayout->addStretch();
    cityHeaderLayout->addWidget(cityTimeLabel);

    scrollLayout->addWidget(cityHeader);



    // Current weather card
    weatherCard = new WeatherCard;
    scrollLayout->addWidget(weatherCard);

    // Weather stats cards
    statsContainer = new QWidget;
    QHBoxLayout *statsLayout = new QHBoxLayout(statsContainer);
    statsLayout->setSpacing(20);

    // Create weather stat cards
    QStringList statTitles = {"Feels Like", "Humidity", "Visibility", "Pressure"};
    QStringList statIcons = {
        "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/temperature.svg",
        "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/humidity.svg",
        "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/visibility.svg",
        "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/pressure.svg"
    };

    for (int i = 0; i < statTitles.size(); i++) {
        QWidget *statCard = new QWidget;
        statCard->setObjectName("statCard");
        QHBoxLayout *cardLayout = new QHBoxLayout(statCard);

        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(QIcon(statIcons[i]).pixmap(32, 32));
        iconLabel->setStyleSheet("background-color: transparent;");

        cardLayout->addWidget(iconLabel);

        QVBoxLayout *textLayout = new QVBoxLayout;
        QLabel *titleLabel = new QLabel(statTitles[i]);
        titleLabel->setObjectName("statTitle");

        QLabel *valueLabel = new QLabel("--");
        valueLabel->setObjectName("statValue");

        textLayout->addWidget(titleLabel);
        textLayout->addWidget(valueLabel);

        cardLayout->addLayout(textLayout);
        cardLayout->addStretch();

        statsLayout->addWidget(statCard);
    }

    scrollLayout->addWidget(statsContainer);

    // Hourly forecast with chart
    QWidget *hourlyContainer = new QWidget;
    hourlyContainer->setObjectName("forecastContainer");
    QVBoxLayout *hourlyLayout = new QVBoxLayout(hourlyContainer);

    QHBoxLayout *hourlyHeaderLayout = new QHBoxLayout;
    QLabel *hourlyTitle = new QLabel("HOURLY FORECAST");
    hourlyTitle->setObjectName("sectionTitle");
    hourlyHeaderLayout->addWidget(hourlyTitle);
    hourlyHeaderLayout->addStretch();

    QPushButton *hourlyExpandBtn = new QPushButton("24 hours");
    hourlyExpandBtn->setObjectName("textButton");
    hourlyHeaderLayout->addWidget(hourlyExpandBtn);

    hourlyLayout->addLayout(hourlyHeaderLayout);

    // Hourly chart
    // Replace the existing hourly chart creation code in createWeatherSection() with this:

    // Hourly chart with proper theming
    QWidget *chartContainer = new QWidget;
    chartContainer->setObjectName("chartContainer");
    chartContainer->setMinimumHeight(180);
    QVBoxLayout *chartLayout = new QVBoxLayout(chartContainer);

    QLineSeries *tempSeries = new QLineSeries();
    tempSeries->append(0, 23); tempSeries->append(1, 22); tempSeries->append(2, 21);
    tempSeries->append(3, 19); tempSeries->append(4, 18); tempSeries->append(5, 17);
    tempSeries->append(6, 16); tempSeries->append(7, 18); tempSeries->append(8, 20);
    tempSeries->append(9, 22); tempSeries->append(10, 24); tempSeries->append(11, 25);

    // Apply themed styling to the line series
    QPen seriesPen;
    if (isDarkTheme) {
        seriesPen = QPen(QColor("#fbbf24"), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    } else {
        seriesPen = QPen(QColor("#e67e22"), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    }
    tempSeries->setPen(seriesPen);

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(tempSeries);
    chart->setTitle("");
    chart->setBackgroundBrush(QBrush(Qt::transparent));
    chart->setPlotAreaBackgroundBrush(QBrush(Qt::transparent));
    chart->setMargins(QMargins(16, 16, 16, 16));

    QCategoryAxis *axisX = new QCategoryAxis();
    axisX->append("12am", 0); axisX->append("3am", 3); axisX->append("6am", 6);
    axisX->append("9am", 9); axisX->append("12pm", 12);
    axisX->setRange(0, 12);

    // Apply themed styling to X axis
    axisX->setGridLineVisible(true);
    axisX->setLineVisible(false);
    if (isDarkTheme) {
        axisX->setGridLineColor(QColor("#334155"));
        axisX->setGridLinePen(QPen(QColor("#334155"), 1, Qt::DashLine));
        axisX->setLabelsColor(QColor("#94a3b8"));
        axisX->setTitleBrush(QBrush(QColor("#94a3b8")));
    } else {
        axisX->setGridLineColor(QColor("#e8e4df"));
        axisX->setGridLinePen(QPen(QColor("#e8e4df"), 1, Qt::DashLine));
        axisX->setLabelsColor(QColor("#627e75"));
        axisX->setTitleBrush(QBrush(QColor("#627e75")));
    }

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(15, 30);
    axisY->setLabelsVisible(false);
    axisY->setGridLineVisible(true);
    axisY->setLineVisible(false);

    // Apply themed styling to Y axis
    if (isDarkTheme) {
        axisY->setGridLineColor(QColor("#334155"));
        axisY->setGridLinePen(QPen(QColor("#334155"), 1, Qt::DashLine));
    } else {
        axisY->setGridLineColor(QColor("#e8e4df"));
        axisY->setGridLinePen(QPen(QColor("#e8e4df"), 1, Qt::DashLine));
    }

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    tempSeries->attachAxis(axisX);
    tempSeries->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setObjectName("chartView");
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setBackgroundBrush(QBrush(Qt::transparent));
    chartView->setFrameShape(QFrame::NoFrame);

    chartLayout->addWidget(chartView);
    hourlyLayout->addWidget(chartContainer);

    // Hourly details
    QWidget *hourlyDetails = new QWidget;
    QHBoxLayout *hourlyDetailsLayout = new QHBoxLayout(hourlyDetails);
    hourlyDetailsLayout->setObjectName("hourlyDetailsLayout");
    hourlyDetailsLayout->setSpacing(0);

    for (int i = 0; i < 6; i++) {
        QWidget *hourCard = new QWidget;
        hourCard->setObjectName("hourCard");
        QVBoxLayout *hourLayout = new QVBoxLayout(hourCard);
        hourLayout->setAlignment(Qt::AlignCenter);
        hourLayout->setContentsMargins(4, 8, 4, 8);
        hourLayout->setSpacing(6);

        hourLayout->addWidget(new QLabel("--"));
        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(getWeatherIcon("", "", false, 32));
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setFixedSize(32, 32);
        hourLayout->addWidget(iconLabel);
        hourLayout->addWidget(new QLabel("--°"));
        hourlyDetailsLayout->addWidget(hourCard, 1);
    }

    hourlyLayout->addWidget(hourlyDetails);
    scrollLayout->addWidget(hourlyContainer);

    // Daily forecast
    ForecastSection *forecastSection = new ForecastSection;
    scrollLayout->addWidget(forecastSection);

    // UV and Air Quality Section
    QWidget *environmentContainer = new QWidget;
    QHBoxLayout *environmentLayout = new QHBoxLayout(environmentContainer);
    environmentLayout->setSpacing(20);

    // UV Index
    QWidget *uvCard = new QWidget;
    uvCard->setObjectName("infoCard");
    QVBoxLayout *uvLayout = new QVBoxLayout(uvCard);

    QLabel *uvTitle = new QLabel("UV INDEX");
    uvTitle->setObjectName("cardTitle");
    QWidget *uvIndicator = new QWidget;
    uvIndicator->setMinimumHeight(36);
    uvIndicator->setObjectName("uvIndicator");
    QLabel *uvValue = new QLabel("--");
    uvValue->setObjectName("uvValue");
    QLabel *uvDesc = new QLabel("--");
    uvDesc->setObjectName("uvDesc");
    uvLayout->addWidget(uvTitle);
    uvLayout->addWidget(uvIndicator);
    uvLayout->addWidget(uvValue);
    uvLayout->addWidget(uvDesc);

    // Air Quality
    QWidget *aqCard = new QWidget;
    aqCard->setObjectName("infoCard");
    QVBoxLayout *aqLayout = new QVBoxLayout(aqCard);
    QLabel *aqTitle = new QLabel("AIR QUALITY");
    aqTitle->setObjectName("cardTitle");
    QWidget *aqIndicator = new QWidget;
    aqIndicator->setMinimumHeight(36);
    aqIndicator->setObjectName("aqIndicator");
    QLabel *aqiValue = new QLabel("--");
    aqiValue->setObjectName("aqiValue");
    QLabel *aqDesc = new QLabel("--");
    aqDesc->setObjectName("aqDesc");
    aqLayout->addWidget(aqTitle);
    aqLayout->addWidget(aqIndicator);
    aqLayout->addWidget(aqiValue);
    aqLayout->addWidget(aqDesc);

    environmentLayout->addWidget(uvCard);
    environmentLayout->addWidget(aqCard);
    scrollLayout->addWidget(environmentContainer);

    // Wind and precipitation details
    QWidget *detailsContainer = new QWidget;
    QHBoxLayout *detailsLayout = new QHBoxLayout(detailsContainer);
    detailsLayout->setSpacing(20);

    // Wind card
    QWidget *windCard = new QWidget;
    windCard->setObjectName("infoCard");
    QVBoxLayout *windLayout = new QVBoxLayout(windCard);
    QLabel *windTitle = new QLabel("WIND");
    windTitle->setObjectName("cardTitle");
    QHBoxLayout *windDataLayout = new QHBoxLayout;
    QLabel *windCompass = new QLabel;
    windCompass->setPixmap(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/compass.svg").pixmap(80, 80));
    windCompass->setAlignment(Qt::AlignCenter);
    QVBoxLayout *windInfoLayout = new QVBoxLayout;
    QLabel *windValue = new QLabel("-- km/h");
    windValue->setObjectName("windValue");
    QLabel *windDirection = new QLabel("--");
    windDirection->setObjectName("windDirection");
    QLabel *gustLabel = new QLabel("Gusts: -- km/h");
    gustLabel->setObjectName("gustLabel");
    windInfoLayout->addWidget(windValue);
    windInfoLayout->addWidget(windDirection);
    windInfoLayout->addWidget(gustLabel);
    windDataLayout->addWidget(windCompass);
    windDataLayout->addLayout(windInfoLayout);
    windLayout->addWidget(windTitle);
    windLayout->addLayout(windDataLayout);

    // Precipitation card
    QWidget *precipCard = new QWidget;
    precipCard->setObjectName("infoCard");
    QVBoxLayout *precipLayout = new QVBoxLayout(precipCard);
    QLabel *precipTitle = new QLabel("PRECIPITATION");
    precipTitle->setObjectName("cardTitle");
    QHBoxLayout *precipDataLayout = new QHBoxLayout;
    QLabel *precipIcon = new QLabel;
    precipIcon->setPixmap(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/rain.svg").pixmap(80, 80));
    precipIcon->setAlignment(Qt::AlignCenter);
    QVBoxLayout *precipInfoLayout = new QVBoxLayout;
    QLabel *precipChance = new QLabel("--%");
    precipChance->setObjectName("precipChance");
    QLabel *precipAmount = new QLabel("-- mm");
    precipAmount->setObjectName("precipAmount");
    QLabel *precipForecast = new QLabel("--");
    precipForecast->setObjectName("precipForecastLabel");
    precipInfoLayout->addWidget(precipChance);
    precipInfoLayout->addWidget(precipAmount);
    precipInfoLayout->addWidget(precipForecast);
    precipDataLayout->addWidget(precipIcon);
    precipDataLayout->addLayout(precipInfoLayout);
    precipLayout->addWidget(precipTitle);
    precipLayout->addLayout(precipDataLayout);

    detailsLayout->addWidget(windCard);
    detailsLayout->addWidget(precipCard);
    scrollLayout->addWidget(detailsContainer);

    // Weather Description
    QWidget *descriptionCard = new QWidget;
    descriptionCard->setObjectName("infoCard");
    QVBoxLayout *descLayout = new QVBoxLayout(descriptionCard);
    QLabel *descTitle = new QLabel("TODAY'S FORECAST");
    descTitle->setObjectName("cardTitle");
    QLabel *weatherDescription = new QLabel("Loading forecast information...");
    weatherDescription->setWordWrap(true);
    weatherDescription->setObjectName("weatherDescription");
    descLayout->addWidget(descTitle);
    descLayout->addWidget(weatherDescription);

    // Weather Briefing Button
    ttsButton = new QPushButton("Weather Briefing");
    ttsButton->setObjectName("textButton");
    ttsButton->setIcon(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/speaker.svg"));
    connect(ttsButton, &QPushButton::clicked, this, &MainWindow::onWeatherBriefingClicked);
    descLayout->addWidget(ttsButton, 0, Qt::AlignRight);
    scrollLayout->addWidget(descriptionCard);
    scrollLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea, 1);
    QTimer::singleShot(100, this, &MainWindow::applyInitialAnimations);
    return weatherWidget;
}

void MainWindow::bounceThemeToggle() {
    QPropertyAnimation *bounce = new QPropertyAnimation(themeToggle, "geometry");
    QRect geom = themeToggle->geometry();
    bounce->setDuration(300);
    bounce->setStartValue(geom);
    bounce->setKeyValueAt(0.5, geom.adjusted(-2, -2, 2, 2));
    bounce->setEndValue(geom);
    bounce->setEasingCurve(QEasingCurve::OutBounce);
    bounce->start(QAbstractAnimation::DeleteWhenStopped);
}


QWidget* MainWindow::createAlertsSection()
{
    return new AlertsSection;
}

void MainWindow::setupSettingsUI()
{
    settingsDialog = new SettingsDialog(this);
}

void MainWindow::setupAnimations()
{
    if(!loadingOverlay) return;

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(loadingOverlay);
    loadingOverlay->setGraphicsEffect(effect);

    fadeAnimation = new QPropertyAnimation(effect, "opacity");
    fadeAnimation->setDuration(300);
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(0.9);
}

void MainWindow::applyTheme()
{
    if (themeToggle) {
        if(isDarkTheme) {
            themeToggle->setIcon(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/sun.svg"));
            themeToggle->setToolTip("Switch to Light Theme");
        } else {
            themeToggle->setIcon(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/moon.svg"));
            themeToggle->setToolTip("Switch to Dark Theme");
        }
        themeToggle->setIconSize(QSize(22, 22));
    }

    QString styleSheet;
    if(isDarkTheme) {
        styleSheet = R"(
        QWidget { background-color: #0a0d1a; color: #e8ecf7; font-family: 'Inter', 'Segoe UI', sans-serif; }
        #forecastsSectionHeader { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1e293b, stop:0.5 #334155, stop:1 #0f172a); border-radius: 12px; margin-bottom: 8px; }
        #forecastsSectionTitle { color: #ffffff; font-size: 24px; font-weight: 600; background: transparent; }
        #forecastsSectionSubtitle { color: #94a3b8; font-size: 14px; background: transparent; }
        #forecastTabs::pane { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #131829, stop:1 #1e293b); border: 1px solid #334155; border-radius: 8px; }
        #forecastTabs::tab-bar { alignment: center; }
        #forecastTabs QTabBar::tab { background: #334155; color: #94a3b8; padding: 12px 24px; margin-right: 4px; border-top-left-radius: 8px; border-top-right-radius: 8px; }
        #forecastTabs QTabBar::tab:selected { background: #1e293b; color: #fbbf24; border-bottom: 2px solid #fbbf24; }
        #forecastTabs QTabBar::tab:hover { background: #475569; color: #ffffff; }
        #forecastScrollArea { background: transparent; }
        #forecastScrollArea QScrollBar:vertical { background: #1a1f35; width: 12px; border-radius: 6px; }
        #forecastScrollArea QScrollBar::handle:vertical { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2d3754, stop:1 #3f4865); border-radius: 6px; min-height: 20px; }
        #forecastScrollArea QScrollBar::handle:vertical:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4a5568, stop:1 #5a6478); }
        #forecastContainer { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1e293b, stop:1 #0f172a); border-radius: 16px; border: 1px solid #334155; }
        #cardHeader { background: transparent; }
        #cardTitle { color: #ffffff; font-size: 20px; font-weight: 600; background: transparent; }
        #cardSubtitle { color: #94a3b8; font-size: 14px; background: transparent; }
        #trendIndicator { color: #fbbf24; font-size: 14px; font-weight: 500; background: transparent; }
        #periodSelector { background: #334155; border-radius: 8px; padding: 8px 16px; }
        #activePeriod { color: #fbbf24; font-weight: 600; font-size: 14px; background: transparent; }
        #chartContainer { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1e293b, stop:0.3 #334155, stop:0.7 #1e293b, stop:1 #0f172a); border-radius: 16px; border: 1px solid #334155; }
        #chartArea { background: transparent; }
        #chartView { background: transparent; border: none; }
        #forecastRow { background: transparent; border: none; border-radius: 14px; }
        #forecastRow:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(124, 156, 255, 0.12), stop:1 rgba(124, 156, 255, 0.06)); }
        #forecastRow[isToday="true"] { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(251, 191, 36, 0.15), stop:1 rgba(251, 191, 36, 0.08)); border: 1px solid rgba(251, 191, 36, 0.25); }
        #dateColumn { background: transparent; }
        #forecastDay { font-family: 'Inter', sans-serif; font-size: 16px; font-weight: 700; color: #e8ecf7; background: transparent; }
        #forecastRow[isToday="true"] #forecastDay { color: #fbbf24; font-weight: 600; font-size: 14px; }
        #forecastDate { font-family: 'Inter', sans-serif; font-size: 14px; font-weight: 500; color: #94a3b8; background: transparent; }
        #iconColumn { background: transparent; }
        #weatherIcon { background: transparent; }
        #conditionColumn { background: transparent; }
        #forecastCondition { font-family: 'Inter', sans-serif; font-size: 15px; font-weight: 600; color: #e8ecf7; background: transparent; }
        #weatherDetail { font-family: 'Inter', sans-serif; font-size: 13px; font-weight: 500; color: #94a3b8; background: transparent; }
        #precipColumn { background: transparent; }
        #precipChance { font-family: 'Inter', sans-serif; font-size: 14px; font-weight: 600; background: transparent; }
        #precipChance[precipLevel="high"] { color: #60a5fa; font-weight: 700; }
        #precipChance[precipLevel="medium"] { color: #fbbf24; font-weight: 600; }
        #precipChance[precipLevel="low"] { color: #94a3b8; font-weight: 500; }
        #tempColumn { background: transparent; }
        #tempLow { font-family: 'Inter', sans-serif; font-size: 17px; font-weight: 600; color: #60a5fa; background: transparent; }
        #tempHigh { font-family: 'Inter', sans-serif; font-size: 17px; font-weight: 700; color: #fbbf24; background: transparent; }
        #tempRangeBar { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #60a5fa, stop:0.25 #93c5fd, stop:0.75 #fbbf24, stop:1 #f59e0b); border-radius: 4px; }
        #rowSeparator { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 transparent, stop:0.15 rgba(124, 156, 255, 0.15), stop:0.85 rgba(124, 156, 255, 0.15), stop:1 transparent); border: none; height: 1px; margin: 12px 28px; }
        #weatherCard, #statCard, #infoCard, #searchContainer, #noAlertsContainer, #alertCardContainer { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1a1f35, stop:1 #242b47); border: 1px solid #2d3754; }
        #headerIcon { color: #7c9cff; background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1e2a5e, stop:1 #2a3554); }
        #weatherCard, #noAlertsContainer, #infoCard { border-radius: 16px; }
        #infoCard { padding: 24px; }
        #statCard, #searchContainer, #alertCardContainer { border-radius: 12px; }
        #statCard { padding: 18px; }
        #refreshIndicator { color: #10b981; }
        #appHeader { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #131829, stop:1 #1a1f35); border-bottom: 1px solid #2d3754; padding: 16px 24px; }
        #appTitle { font-size: 22px; font-weight: 600; margin-left: 12px; color: #7c9cff; background: transparent; }
        #sideNav { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #131829, stop:1 #1a1f35); border-right: 1px solid #2d3754; }
        #navSectionTitle { color: #6b7eb8; font-size: 11px; font-weight: 700; text-transform: uppercase; letter-spacing: 1px; margin-top: 24px; margin-bottom: 8px; background: transparent; }
        #navButton { text-align: left; padding: 12px 18px; border-radius: 8px; font-weight: 500; background: transparent; border: none; color: #b4c6ef; }
        #navButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2a3554, stop:1 #34426b); }
        #navButton:checked { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4c6fff, stop:1 #7c9cff); color: white; }
        #contentWrapper { background-color: #0a0d1a; }
        #sectionTitle { font-size: 20px; font-weight: 700; color: #e8ecf7; margin-bottom: 20px; background: transparent; }
        #weatherCard { background-color: #0a0d1a; background-position: top right; background-repeat: no-repeat; }
        QLabel, #cityHeaderWidget, #hourlyDetailsLayout > QWidget { background: transparent; border: none; }
        #cityNameLabel, #cityTimeLabel, #tempLabel, #conditionLabel, #dateLabel, #highLowLabel, #sunTimeLabel, #statTitle, #statValue, #forecastHigh, #forecastLow, #windValue, #windDirection, #gustLabel, #precipAmount, #precipForecastLabel, #weatherDescription, #uvValue, #aqiValue, #uvDesc, #aqDesc { background-color: transparent; }
        #tempLabel { font-size: 78px; font-weight: 200; color: #e8ecf7; }
        #conditionLabel { font-size: 22px; color: #7c9cff; margin-top: -12px; font-weight: 500; }
        #dateLabel { font-size: 16px; color: #9bb0d9; font-weight: 400; }
        #highLowLabel { font-size: 18px; color: #e8ecf7; font-weight: 500; }
        #sunTimeLabel { font-size: 14px; color: #ffa726; font-weight: 500; }
        #statTitle { font-size: 13px; color: #6b7eb8; font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px; }
        #statValue { font-size: 28px; font-weight: 600; color: #e8ecf7; }
        #dayForecastCard { padding: 16px 8px; border-radius: 10px; }
        #dayForecastCard:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2a3554, stop:1 #34426b); }
        #forecastHigh { font-size: 19px; font-weight: 600; color: #e8ecf7; }
        #forecastLow { font-size: 16px; color: #9bb0d9; font-weight: 400; }
        QChart { color: #b4c6ef; }
        QCategoryAxis, QValueAxis { grid-line-color: #2d3754; line-pen-color: #434e73; labels-color: #9bb0d9; }
        #windValue { font-size: 28px; font-weight: 600; color: #e8ecf7; }
        #windDirection { font-size: 18px; color: #7c9cff; font-weight: 500; }
        #gustLabel { font-size: 16px; color: #7c9cff; }
        #precipAmount { font-size: 18px; color: #66d9ff; font-weight: 500; }
        #weatherDescription { font-size: 16px; line-height: 1.6; color: #b4c6ef; font-weight: 400; }
        #primaryButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4c6fff, stop:1 #7c9cff); color: white; border: none; padding: 12px 24px; border-radius: 8px; font-weight: 600; }
        #primaryButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #5c7fff, stop:1 #8cacff); }
        #secondaryButton { background-color: transparent; color: #7c9cff; border: 2px solid #4c6fff; padding: 10px 20px; border-radius: 8px; font-weight: 600; }
        #secondaryButton:hover { background-color: rgba(76, 111, 255, 0.15); border-color: #7c9cff; }
        #textButton { background-color: transparent; color: #7c9cff; border: none; padding: 6px 12px; font-weight: 600; }
        #textButton:hover { text-decoration: underline; color: #8cacff; }
        #iconButton { background-color: rgba(42, 53, 84, 0.7); border: none; border-radius: 50%; padding: 8px; }
        #iconButton:hover { background-color: rgba(124, 156, 255, 0.15); }
        #searchInput { background-color: transparent; border: none; font-size: 16px; padding: 12px; color: #e8ecf7; }
        #searchInput::placeholder { color: #6b7eb8; }
        #settingsDialog { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1a1f35, stop:1 #242b47); }
        #settingsTitle { font-size: 18px; font-weight: 700; color: #e8ecf7; margin-bottom: 12px; background: transparent; }
        #settingsCombo { background-color: #2d3754; color: #e8ecf7; border: 1px solid #434e73; padding: 10px; border-radius: 8px; min-width: 160px; }
        #colorButton { border-radius: 8px; border: 1px solid #434e73; }
        #statusBar { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1a1f35, stop:1 #242b47); border-top: 1px solid #2d3754; font-size: 12px; color: #6b7eb8; }
        #uvIndicator { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6a4c93, stop:0.5 #9c89b3, stop:1 #c8b2db); border-radius: 6px; }
        #aqIndicator { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4c6fff, stop:0.5 #7c9cff, stop:1 #a8c0ff); border-radius: 6px; }
        #uvDesc, #aqDesc { font-size: 16px; color: #7c9cff; font-weight: 500; }
        #hourCard { padding: 12px; border-radius: 8px; }
        #hourCard:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2a3554, stop:1 #34426b); }
        #alertsScrollArea { background-color: transparent; border: none; }
        #alertsContainer { background-color: transparent; }
        #alertCountBadge { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4c6fff, stop:1 #7c9cff); color: white; border-radius: 12px; padding: 4px 12px; font-weight: 600; font-size: 13px; }
        #alertCardContainer[alertSeverity="severe"] { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #331a24, stop:1 #3d1f2b); border: 1px solid #4d2935; }
        #alertCardContainer[alertSeverity="moderate"] { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #332815, stop:1 #3d2e19); border: 1px solid #4d3921; }
        #alertTitle { font-size: 18px; font-weight: 600; color: #e8ecf7; background-color: transparent; }
        #alertCardContainer[alertSeverity="severe"] #alertTitle { color: #ff4d6a; }
        #alertCardContainer[alertSeverity="moderate"] #alertTitle { color: #ffa726; }
        #alertSource { font-size: 13px; color: #6b7eb8; background-color: transparent; }
        #alertTime { font-size: 14px; color: #7c9cff; background-color: transparent; margin-top: 4px; }
        #alertCardContainer[alertSeverity="severe"] #alertTime { color: #ff8fa3; }
        #alertCardContainer[alertSeverity="moderate"] #alertTime { color: #ffc166; }
        #alertDescription { font-size: 14px; line-height: 1.6; color: #b4c6ef; background-color: transparent; padding: 8px 0; }
        #alertExpandButton { background-color: transparent; border: none; border-radius: 50%; padding: 4px; }
        #alertExpandButton:hover { background-color: rgba(124, 156, 255, 0.15); }
        #alertDetailsContainer { background-color: transparent; }
        #noAlertsTitle { font-size: 22px; font-weight: 600; color: #7c9cff; background-color: transparent; }
        #noAlertsDescription { font-size: 16px; color: #6b7eb8; background-color: transparent; }
        #mapHeaderCard { background: transparent; border: 1px solid #2d3754; }
        #mapCard { background: transparent; border: 1px solid #2d3754; }
        #mapSubtitle, #mapComboLabel, #mapLayerInfo { color: #8B9CD6; font-size: 14px; background: transparent; }
        #mapTypeCombo { background: #242b47; border: 1px solid #2d3754; border-radius: 8px; padding: 8px 12px; color: #e8ecf7; }
        #mapStatusBar { background: rgba(45, 55, 84, 0.5); border-bottom: 1px solid #2d3754; }
        #mapStatusIndicator { color: #6b7eb8; }
        #mapStatusIndicatorLoading { color: #f59e0b; }
        #mapStatusIndicatorReady { color: #10b981; }
        #mapStatusIndicatorError { color: #ef4444; }
        #mapLegendCard { background: rgba(26, 31, 53, 0.6); border: 1px solid #2d3754; }
        #mapPlaceholder { background: rgba(26, 31, 53, 0.3); border-radius: 12px; border: 2px dashed #2d3754; }
        #mapPlaceholderIcon { font-size: 48px; }
        #mapPlaceholderText { font-size: 18px; font-weight: 600; color: #b4c6ef; }
        #mapPlaceholderHint { font-size: 14px; color: #8B9CD6; }
    )";
    } else {
        styleSheet = R"(
        QWidget { background-color: #f8f5f2; color: #3d3a36; font-family: 'Inter', 'Segoe UI', sans-serif; }
        #forecastsSectionHeader { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #f1ede8, stop:0.5 #e8e4df, stop:1 #ffffff); border-radius: 12px; margin-bottom: 8px; border: 1px solid #e8e4df; }
        #forecastsSectionTitle { color: #3d3a36; font-size: 24px; font-weight: 600; background: transparent; }
        #forecastsSectionSubtitle { color: #627e75; font-size: 14px; background: transparent; }
        #forecastTabs::pane { background: #ffffff; border: 1px solid #e8e4df; border-radius: 8px; }
        #forecastTabs::tab-bar { alignment: center; }
        #forecastTabs QTabBar::tab { background: #f1ede8; color: #627e75; padding: 12px 24px; margin-right: 4px; border-top-left-radius: 8px; border-top-right-radius: 8px; border: 1px solid #e8e4df; }
        #forecastTabs QTabBar::tab:selected { background: #ffffff; color: #226ba3; border-bottom: 2px solid #226ba3; }
        #forecastTabs QTabBar::tab:hover { background: #e8e4df; color: #3d3a36; }
        #forecastScrollArea { background: transparent; }
        #forecastScrollArea QScrollBar:vertical { background: #f1ede8; width: 12px; border-radius: 6px; }
        #forecastScrollArea QScrollBar::handle:vertical { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e8e4df, stop:1 #d4cfc8); border-radius: 6px; min-height: 20px; }
        #forecastScrollArea QScrollBar::handle:vertical:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #d4cfc8, stop:1 #c0bab1); }
        #forecastContainer { background: #ffffff; border-radius: 16px; border: 1px solid #e8e4df; }
        #cardHeader { background: transparent; }
        #cardTitle { color: #3d3a36; font-size: 20px; font-weight: 600; background: transparent; }
        #cardSubtitle { color: #627e75; font-size: 14px; background: transparent; }
        #trendIndicator { color: #e67e22; font-size: 14px; font-weight: 500; background: transparent; }
        #periodSelector { background: #f1ede8; border-radius: 8px; padding: 8px 16px; border: 1px solid #e8e4df; }
        #activePeriod { color: #226ba3; font-weight: 600; font-size: 14px; background: transparent; }
        #chartContainer { background: #ffffff; border-radius: 16px; border: 1px solid #e8e4df; }
        #chartArea { background: transparent; }
        #chartView { background: transparent; border: none; }
        #forecastRow { background: transparent; border: none; border-radius: 14px; }
        #forecastRow:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(34, 107, 163, 0.08), stop:1 rgba(34, 107, 163, 0.04)); }
        #forecastRow[isToday="true"] { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(230, 126, 34, 0.1), stop:1 rgba(230, 126, 34, 0.05)); border: 1px solid rgba(230, 126, 34, 0.2); }
        #dateColumn { background: transparent; }
        #forecastDay { font-family: 'Inter', sans-serif; font-size: 16px; font-weight: 700; color: #3d3a36; background: transparent; }
        #forecastRow[isToday="true"] #forecastDay { color: #e67e22; font-weight: 600; font-size: 14px; }
        #forecastDate { font-family: 'Inter', sans-serif; font-size: 14px; font-weight: 500; color: #627e75; background: transparent; }
        #iconColumn { background: transparent; }
        #weatherIcon { background: transparent; }
        #conditionColumn { background: transparent; }
        #forecastCondition { font-family: 'Inter', sans-serif; font-size: 15px; font-weight: 600; color: #3d3a36; background: transparent; }
        #weatherDetail { font-family: 'Inter', sans-serif; font-size: 13px; font-weight: 500; color: #627e75; background: transparent; }
        #precipColumn { background: transparent; }
        #precipChance { font-family: 'Inter', sans-serif; font-size: 14px; font-weight: 600; background: transparent; }
        #precipChance[precipLevel="high"] { color: #2980b9; font-weight: 700; }
        #precipChance[precipLevel="medium"] { color: #e67e22; font-weight: 600; }
        #precipChance[precipLevel="low"] { color: #627e75; font-weight: 500; }
        #tempColumn { background: transparent; }
        #tempLow { font-family: 'Inter', sans-serif; font-size: 17px; font-weight: 600; color: #3498db; background: transparent; }
        #tempHigh { font-family: 'Inter', sans-serif; font-size: 17px; font-weight: 700; color: #e67e22; background: transparent; }
        #tempRangeBar { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3498db, stop:0.25 #5dade2, stop:0.75 #e67e22, stop:1 #d35400); border-radius: 4px; }
        #rowSeparator { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 transparent, stop:0.15 rgba(34, 107, 163, 0.12), stop:0.85 rgba(34, 107, 163, 0.12), stop:1 transparent); border: none; height: 1px; margin: 12px 28px; }
        #weatherCard, #statCard, #infoCard, #searchContainer, #noAlertsContainer, #alertCardContainer { background-color: #ffffff; border: 1px solid #e8e4df; }
        #weatherCard, #noAlertsContainer, #infoCard { border-radius: 16px; }
        #infoCard { padding: 24px; }
        #statCard, #searchContainer, #alertCardContainer { border-radius: 12px; }
        #statCard { padding: 18px; }
        #appHeader, #sideNav { background-color: #f1ede8; }
        #appHeader { border-bottom: 1px solid #e8e4df; padding: 16px 24px; }
        #sideNav { border-right: 1px solid #e8e4df; }
        #appTitle { font-size: 22px; font-weight: 700; margin-left: 12px; color: #226ba3; background: transparent; }
        #navSectionTitle { color: #627e75; font-size: 11px; font-weight: 700; text-transform: uppercase; letter-spacing: 1px; margin-top: 24px; margin-bottom: 8px; background: transparent; }
        #navButton { text-align: left; padding: 12px 18px; border-radius: 8px; font-weight: 500; background: transparent; border: none; color: #627e75; }
        #navButton:hover { background-color: #e8e4df; color: #226ba3; }
        #navButton:checked { background-color: #226ba3; color: white; }
        #contentWrapper { background-color: #f8f5f2; }
        #sectionTitle { font-size: 20px; font-weight: 700; color: #3d3a36; margin-bottom: 20px; background: transparent; }
        #weatherCard { background-color: #f8f5f2; background-position: top right; background-repeat: no-repeat; }
        QLabel { background: transparent; border: none; }
        #tempLabel { font-size: 78px; font-weight: 200; color: #3d3a36; }
        #conditionLabel { font-size: 22px; color: #627e75; margin-top: -12px; font-weight: 500; }
        #highLowLabel { font-size: 18px; color: #3d3a36; font-weight: 500; }
        #sunTimeLabel { font-size: 14px; color: #d2a985; font-weight: 600; }
        #statTitle, #cardTitle { font-size: 13px; color: #627e75; font-weight: 700; text-transform: uppercase; }
        #statValue { font-size: 28px; font-weight: 600; color: #3d3a36; }
        #weatherDescription { font-size: 16px; line-height: 1.6; color: #627e75; }
        #primaryButton { background-color: #226ba3; color: white; border: none; padding: 12px 24px; border-radius: 8px; font-weight: 600; }
        #primaryButton:hover { background-color: #1e5a8a; }
        #secondaryButton { background-color: transparent; color: #226ba3; border: 2px solid #226ba3; padding: 10px 20px; border-radius: 8px; font-weight: 600; }
        #secondaryButton:hover { background-color: rgba(34, 107, 163, 0.1); }
        #textButton { background-color: transparent; color: #226ba3; border: none; font-weight: 600; }
        #textButton:hover { text-decoration: underline; }
        #iconButton { background-color: #e8e4df; border: none; border-radius: 50%; padding: 8px; }
        #iconButton:hover { background-color: rgba(34, 107, 163, 0.1); }
        #searchInput { background-color: transparent; border: none; font-size: 16px; padding: 12px; color: #3d3a36; }
        #searchInput::placeholder { color: #8a8178; }
        #alertCardContainer[alertSeverity="severe"] { background-color: #fdeaea; border: 1px solid #d99596; }
        #alertCardContainer[alertSeverity="severe"] #alertTitle { color: #913639; }
        #alertCardContainer[alertSeverity="moderate"] { background-color: #fff8e1; border: 1px solid #e9c429; }
        #alertCardContainer[alertSeverity="moderate"] #alertTitle { color: #627e75; }
        #alertTitle, #alertSource, #alertTime, #alertDescription { background-color: transparent; }
        #noAlertsTitle { color: #226ba3; background-color: transparent; }
        #noAlertsDescription { color: #627e75; background-color: transparent; }
        #statusBar { background-color: #f1ede8; border-top: 1px solid #e8e4df; font-size: 12px; color: #627e75; }
        #settingsDialog { background-color: #f8f5f2; }
        #settingsTitle { color: #3d3a36; background: transparent; }
        #settingsCombo { background-color: #ffffff; color: #3d3a36; border: 1px solid #e8e4df; padding: 10px; border-radius: 8px; }
        #colorButton { border: 1px solid #e8e4df; }
        #dateLabel, #cityNameLabel, #cityTimeLabel, #windValue, #windDirection, #gustLabel, #precipAmount, #precipForecastLabel, #uvValue, #aqiValue, #uvDesc, #aqDesc, #forecastHigh, #forecastLow { background-color: transparent; }
        #windValue { font-size: 28px; font-weight: 600; color: #3d3a36; }
        #windDirection { font-size: 18px; color: #226ba3; font-weight: 500; }
        #gustLabel { font-size: 16px; color: #226ba3; }
        #precipAmount { font-size: 18px; color: #2980b9; font-weight: 500; }
        #uvDesc, #aqDesc { font-size: 16px; color: #226ba3; font-weight: 500; }
        #forecastHigh { font-size: 19px; font-weight: 600; color: #3d3a36; }
        #forecastLow { font-size: 16px; color: #627e75; font-weight: 400; }
        #hourCard { padding: 12px; border-radius: 8px; }
        #hourCard:hover { background: #f1ede8; }
        #dayForecastCard { padding: 16px 8px; border-radius: 10px; }
        #dayForecastCard:hover { background: #f1ede8; }
        QChart { color: #627e75; }
        QCategoryAxis, QValueAxis { grid-line-color: #e8e4df; line-pen-color: #d4cfc8; labels-color: #627e75; }
        #uvIndicator { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #9b59b6, stop:0.5 #bb8fce, stop:1 #d7bde2); border-radius: 6px; }
        #aqIndicator { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #226ba3, stop:0.5 #5dade2, stop:1 #aed6f1); border-radius: 6px; }
        #mapHeaderCard { background: transparent; border: 1px solid #e8e4df; }
        #mapCard { background: transparent; border: 1px solid #e8e4df; }
        #mapSubtitle, #mapComboLabel, #mapLayerInfo { color: #627e75; font-size: 14px; background: transparent; }
        #mapTypeCombo { background: #ffffff; border: 1px solid #e8e4df; border-radius: 8px; padding: 8px 12px; color: #3d3a36; }
        #mapStatusBar { background: rgba(241, 237, 232, 0.8); border-bottom: 1px solid #e8e4df; }
        #mapStatusIndicator { color: #627e75; }
        #mapStatusIndicatorLoading { color: #e67e22; }
        #mapStatusIndicatorReady { color: #27ae60; }
        #mapStatusIndicatorError { color: #e74c3c; }
        #mapLegendCard { background: rgba(248, 245, 242, 0.9); border: 1px solid #e8e4df; }
        #mapPlaceholder { background: rgba(248, 245, 242, 0.5); border-radius: 12px; border: 2px dashed #e8e4df; }
        #mapPlaceholderIcon { font-size: 48px; }
        #mapPlaceholderText { font-size: 18px; font-weight: 600; color: #627e75; }
        #mapPlaceholderHint { font-size: 14px; color: #8a8178; }
    )";
    }
    qApp->setStyleSheet(styleSheet);
}
void MainWindow::showSection(const QString &ion)
{
    if(ion == "Settings") {
        settingsDialog->exec();
        return;
    }

    int index = navButtons.indexOf(qobject_cast<QPushButton*>(sender()));
    if(index >= 0) {
        mainStack->setCurrentIndex(index);
        for(QPushButton *btn : navButtons) btn->setChecked(false);
        qobject_cast<QPushButton*>(sender())->setChecked(true);
    }
}


void MainWindow::showSection(int index) {
    if (index < 0 || index >= mainStack->count()) return;

    QWidget *current = mainStack->currentWidget();
    QWidget *target = mainStack->widget(index);

    if (current == target) return;

    QGraphicsOpacityEffect *fadeOutEffect = new QGraphicsOpacityEffect(current);
    current->setGraphicsEffect(fadeOutEffect);
    QPropertyAnimation *fadeOut = new QPropertyAnimation(fadeOutEffect, "opacity");
    fadeOut->setDuration(250);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    QGraphicsOpacityEffect *fadeInEffect = new QGraphicsOpacityEffect(target);
    target->setGraphicsEffect(fadeInEffect);
    QPropertyAnimation *fadeIn = new QPropertyAnimation(fadeInEffect, "opacity");
    fadeIn->setDuration(250);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    connect(fadeOut, &QPropertyAnimation::finished, [=]() {
        mainStack->setCurrentWidget(target);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);

    for (QPushButton *btn : navButtons) btn->setChecked(false);
    navButtons[index]->setChecked(true);
}

void MainWindow::applyInitialAnimations() {
    // Weather section initial animation
    animateWidgetEntrance(weatherCard, 0);
    animateWidgetEntrance(statsContainer, 150);
    animateWidgetEntrance(ttsButton, 250);
}

void MainWindow::onSearchClicked() {
    QString city = cityInput->text().trimmed();
    if (city.isEmpty()) return;
    showLoadingOverlay(true);
    statusLabel->setText(QString("Finding coordinates for %1...").arg(city));

    QNetworkAccessManager *geoManager = new QNetworkAccessManager(this);
    connect(geoManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *geoReply) {
        if (geoReply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(geoReply->readAll());
            if (doc.isArray() && !doc.array().isEmpty()) {
                QJsonObject cityData = doc.array()[0].toObject();
                double lat = cityData["lat"].toDouble();
                double lon = cityData["lon"].toDouble();

                MapsSection *mapSection = findChild<MapsSection *>();
                if (mapSection) {
                    mapSection->fetchMap(lat, lon);
                }

                statusLabel->setText("Fetching full weather data...");

                QNetworkAccessManager *weatherManager = new QNetworkAccessManager(this);
                connect(weatherManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *weatherReply) {
                    showLoadingOverlay(false);
                    if (weatherReply->error() == QNetworkReply::NoError) {
                        handleWeatherData(QJsonDocument::fromJson(weatherReply->readAll()).object());
                    } else { QMessageBox::critical(this, "Weather API Error", weatherReply->errorString()); }
                    weatherReply->deleteLater(); weatherManager->deleteLater();
                });
                QUrl oneCallUrl("https://api.openweathermap.org/data/3.0/onecall");
                QUrlQuery oneCallQuery;
                oneCallQuery.addQueryItem("lat", QString::number(lat)); oneCallQuery.addQueryItem("lon", QString::number(lon));
                oneCallQuery.addQueryItem("appid", API_KEY); oneCallQuery.addQueryItem("units", "metric");
                oneCallUrl.setQuery(oneCallQuery);
                weatherManager->get(QNetworkRequest(oneCallUrl));

                QNetworkAccessManager *aqiManager = new QNetworkAccessManager(this);
                connect(aqiManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *aqiReply) {
                    if (aqiReply->error() == QNetworkReply::NoError) {
                        QJsonDocument aqiDoc = QJsonDocument::fromJson(aqiReply->readAll());
                        int aqi = aqiDoc.object()["list"].toArray()[0].toObject()["main"].toObject()["aqi"].toInt();
                        findChild<QLabel*>("aqiValue")->setText(QString::number(aqi));
                        QString aqiDesc;
                        if (aqi == 1) aqiDesc = "Good"; else if (aqi == 2) aqiDesc = "Fair"; else if (aqi == 3) aqiDesc = "Moderate";
                        else if (aqi == 4) aqiDesc = "Poor"; else aqiDesc = "Very Poor";
                        findChild<QLabel*>("aqDesc")->setText(aqiDesc);
                    }
                    aqiReply->deleteLater(); aqiManager->deleteLater();
                });
                QUrl aqiUrl("https://api.openweathermap.org/data/2.5/air_pollution");
                QUrlQuery aqiQuery;
                aqiQuery.addQueryItem("lat", QString::number(lat)); aqiQuery.addQueryItem("lon", QString::number(lon));
                aqiQuery.addQueryItem("appid", API_KEY);
                aqiUrl.setQuery(aqiQuery);
                aqiManager->get(QNetworkRequest(aqiUrl));
            } else { showLoadingOverlay(false); QMessageBox::warning(this, "Geocoding Error", "City not found."); }
        } else { showLoadingOverlay(false); QMessageBox::critical(this, "Geocoding API Error", geoReply->errorString()); }
        geoReply->deleteLater(); geoManager->deleteLater();
    });
    QUrl geoUrl("https://api.openweathermap.org/geo/1.0/direct");
    QUrlQuery geoQuery;
    geoQuery.addQueryItem("q", city); geoQuery.addQueryItem("limit", "1"); geoQuery.addQueryItem("appid", API_KEY);
    geoUrl.setQuery(geoQuery);
    geoManager->get(QNetworkRequest(geoUrl));
}

void MainWindow::handleWeatherData(const QJsonObject &data) {
    if (!data.contains("current")) {
        QMessageBox::warning(this, "API Error", "Received incomplete weather data from OneCall API.");
        return;
    }
    QJsonObject current = data.value("current").toObject();
    QJsonArray daily = data.value("daily").toArray();
    QJsonObject dailyToday = daily[0].toObject();

    QMap<QString, QString> iconMap;

    iconMap["01d"] = "clear-day.svg";
    iconMap["02d"] = "partly-cloudy-day.svg";
    iconMap["03d"] = "cloudy.svg";
    iconMap["04d"] = "overcast.svg";
    iconMap["09d"] = "showers.svg";
    iconMap["10d"] = "rain.svg";
    iconMap["11d"] = "thunderstorms.svg";
    iconMap["13d"] = "snow.svg";
    iconMap["50d"] = "fog.svg";

    iconMap["01n"] = "clear-night.svg";
    iconMap["02n"] = "partly-cloudy-night.svg";
    iconMap["03n"] = "cloudy.svg";
    iconMap["04n"] = "overcast.svg";
    iconMap["09n"] = "showers.svg";
    iconMap["10n"] = "rain.svg";
    iconMap["11n"] = "thunderstorms.svg";
    iconMap["13n"] = "snow.svg";
    iconMap["50n"] = "fog.svg";

    WeatherCard *wc = findChild<WeatherCard*>();
    if (!wc) return; // Safety check

    QString currentIconCode = current.value("weather").toArray()[0].toObject().value("icon").toString();
    QString iconFile = iconMap.value(currentIconCode, "partly-cloudy.svg"); // Default if not found
    QString iconPath = "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/weather/" + iconFile;
    wc->conditionIcon->setPixmap(QPixmap(iconPath).scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    findChild<QLabel*>("cityNameLabel")->setText(cityInput->text());
    QDateTime date = QDateTime::fromSecsSinceEpoch(current.value("dt").toInteger());
    findChild<QLabel*>("dateLabel")->setText(date.toString("dddd, MMMM d"));
    wc->tempLabel->setText(QString::number(qRound(current.value("temp").toDouble())) + "°");
    QString condition = current.value("weather").toArray()[0].toObject().value("description").toString();
    condition[0] = condition[0].toUpper();
    wc->conditionLabel->setText(condition);
    findChild<QLabel*>("highLowLabel")->setText(QString("H: %1° L: %2°").arg(qRound(dailyToday.value("temp").toObject().value("max").toDouble())).arg(qRound(dailyToday.value("temp").toObject().value("min").toDouble())));
    wc->sunriseLabel->setText(QDateTime::fromSecsSinceEpoch(current.value("sunrise").toInteger()).toString("h:mm AP"));
    wc->sunsetLabel->setText(QDateTime::fromSecsSinceEpoch(current.value("sunset").toInteger()).toString("h:mm AP"));

    QList<QWidget*> statCards = findChildren<QWidget*>("statCard");
    for(QWidget* card : statCards) {
        QLabel* title = card->findChild<QLabel*>("statTitle");
        QLabel* value = card->findChild<QLabel*>("statValue");
        if (!title || !value) continue;
        if(title->text() == "Feels Like") value->setText(QString::number(qRound(current.value("feels_like").toDouble())) + "°");
        else if(title->text() == "Humidity") value->setText(QString::number(current.value("humidity").toInt()) + "%");
        else if(title->text() == "Visibility") value->setText(QString::number(current.value("visibility").toInt() / 1000.0, 'f', 1) + " km");
        else if(title->text() == "Pressure") value->setText(QString::number(current.value("pressure").toInt()) + " hPa");
    }

    findChild<QLabel*>("windValue")->setText(QString::number(current.value("wind_speed").toDouble() * 3.6, 'f', 1) + " km/h");
    double gustSpeed = current.contains("wind_gust") ? current.value("wind_gust").toDouble() * 3.6 : 0.0;
    findChild<QLabel*>("gustLabel")->setText("Gusts: " + QString::number(gustSpeed, 'f', 1) + " km/h");
    findChild<QLabel*>("precipChance")->setText(QString::number(dailyToday.value("pop").toDouble() * 100, 'f', 0) + "%");
    double rainAmount = dailyToday.contains("rain") ? dailyToday.value("rain").toDouble() : 0.0;
    findChild<QLabel*>("precipAmount")->setText(QString::number(rainAmount, 'f', 1) + " mm");

    double uvi = current.value("uvi").toDouble();
    findChild<QLabel*>("uvValue")->setText(QString::number(uvi, 'f', 1));
    QString uvDesc;
    if (uvi < 3) uvDesc = "Low"; else if (uvi < 6) uvDesc = "Moderate"; else if (uvi < 8) uvDesc = "High";
    else if (uvi < 11) uvDesc = "Very High"; else uvDesc = "Extreme";
    findChild<QLabel*>("uvDesc")->setText(uvDesc);

    QHBoxLayout* hourlyLayout = findChild<QHBoxLayout*>("hourlyDetailsLayout");
    if(hourlyLayout) {
        QLayoutItem* item;
        while ((item = hourlyLayout->takeAt(0)) != nullptr) { delete item->widget(); delete item; }
        QJsonArray hourly = data.value("hourly").toArray();
        for(int i = 0; i < 12 && i < hourly.size(); ++i) {
            QJsonObject hourData = hourly[i].toObject();
            QWidget *hourCard = new QWidget;
            hourCard->setObjectName("hourCard");
            QVBoxLayout *cardLayout = new QVBoxLayout(hourCard);
            cardLayout->setAlignment(Qt::AlignCenter);
            cardLayout->addWidget(new QLabel(i == 0 ? "Now" : QDateTime::fromSecsSinceEpoch(hourData.value("dt").toInteger()).toString("ha")));

            QLabel* iconLabel = new QLabel();
            QString hourlyIconCode = hourData.value("weather").toArray()[0].toObject().value("icon").toString();
            QString hourlyIconFile = iconMap.value(hourlyIconCode, "partly-cloudy.svg");
            QString hourlyIconPath = "C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/weather/" + hourlyIconFile;
            iconLabel->setPixmap(QPixmap(hourlyIconPath).scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            cardLayout->addWidget(iconLabel);

            cardLayout->addWidget(new QLabel(QString::number(qRound(hourData.value("temp").toDouble())) + "°"));
            hourlyLayout->addWidget(hourCard, 1);
        }
    }

    ForecastSection *forecastSection = findChild<ForecastSection*>();
    if (forecastSection) {
        forecastSection->clearForecasts();
        QJsonArray dailyArray = data.value("daily").toArray();
        for (int i = 1; i < dailyArray.size(); ++i) {
            QJsonObject dayData = dailyArray[i].toObject();
            forecastSection->addDayForecast(
                QDateTime::fromSecsSinceEpoch(dayData.value("dt").toInteger()).toString("ddd"),
                dayData.value("weather").toArray()[0].toObject().value("icon").toString(),
                QString::number(qRound(dayData.value("temp").toObject().value("max").toDouble())) + "°",
                QString::number(qRound(dayData.value("temp").toObject().value("min").toDouble())) + "°"
                );
        }
    }

    ForecastsSection *extendedForecast = findChild<ForecastsSection*>();
    if (extendedForecast) {
        extendedForecast->updateForecasts(daily);
    }

    QString summary = dailyToday.value("summary").toString();
    findChild<QLabel*>("weatherDescription")->setText(summary);

    AlertsSection *alertsSection = findChild<AlertsSection*>();
    if (alertsSection) {
        if (data.contains("alerts")) {
            alertsSection->updateAlerts(data.value("alerts").toArray());
        } else {
            alertsSection->updateAlerts(QJsonArray());
        }
    }

    statusLabel->setText("Weather data updated for " + cityInput->text());
}

void MainWindow::updateCityTime()
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        //not implemented yet
    });
    timer->start(1000);
}

void MainWindow::showLoadingOverlay(bool show)
{
    if(!loadingOverlay) {
        loadingOverlay = new QWidget(this);
        loadingOverlay->setGeometry(rect());
        loadingOverlay->setAutoFillBackground(true);

        QVBoxLayout *layout = new QVBoxLayout(loadingOverlay);
        QMovie *spinner = new QMovie("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/spinner.gif");
        QLabel *loader = new QLabel;
        loader->setMovie(spinner);
        spinner->start();

        QLabel *loadingText = new QLabel("Loading weather data...");
        loadingText->setAlignment(Qt::AlignCenter);

        layout->addWidget(loader, 0, Qt::AlignCenter);
        layout->addWidget(loadingText, 0, Qt::AlignCenter);
    }

    loadingOverlay->resize(size());
    if(show) {
        loadingOverlay->raise();
        loadingOverlay->show();
        if(fadeAnimation) fadeAnimation->start();
    }
    else {
        loadingOverlay->hide();
    }
}

void MainWindow::toggleTheme()
{
    isDarkTheme = !isDarkTheme;
    applyTheme();
    SettingsManager::saveSettings(isDarkTheme ? "dark" : "light", cityInput->text());
}

void MainWindow::updateUnits()
{}

void MainWindow::saveSettings()
{
    QJsonObject settings;
    settings["theme"] = isDarkTheme ? "dark" : "light";
    settings["lastCity"] = cityInput->text();
    settings["tempUnit"] = settingsDialog->tempUnitCombo->currentText();
    settings["windUnit"] = settingsDialog->windUnitCombo->currentText();
    settings["pressureUnit"] = settingsDialog->pressureUnitCombo->currentText();

    SettingsManager::saveSettings(isDarkTheme ? "dark" : "light", cityInput->text());
    settingsDialog->accept();
    statusLabel->setText("Settings saved successfully");
}

void MainWindow::useCurrentLocation()
{
    statusLabel->setText("Detecting your location...");
    QTimer::singleShot(1500, this, [this]() {
        cityInput->setText("Current Location");
        onSearchClicked();
    });
}

void MainWindow::onWeatherBriefingClicked()
{
    if (!speechEngine || !ttsButton) return;


    if (speechEngine->state() == QTextToSpeech::Speaking) {
        ttsButton->setEnabled(false);
        speechEngine->stop();
    }

    else if (speechEngine->state() == QTextToSpeech::Ready) {
        QString briefing = generateWeatherBriefingText();
        if (briefing == "No weather data available to read.") {
            QMessageBox::warning(this, "No Data", briefing);
            return;
        }
        ttsButton->setEnabled(false);
        speechEngine->say(briefing);
    }
}


void MainWindow::onTtsStateChanged(QTextToSpeech::State state)
{
    if (!ttsButton) return;

    if (state == QTextToSpeech::Speaking) {
        ttsButton->setText("Stop Briefing");
        ttsButton->setIcon(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/cancel.svg"));
        ttsButton->setEnabled(true);
    } else {
        ttsButton->setText("Weather Briefing");
        ttsButton->setIcon(QIcon("C:/Users/ALLI/Documents/Study/Sem2/OOP/Project/Zephyr-main/Zephyr-main/resources/icons/speaker.svg"));
        ttsButton->setEnabled(true);
    }
}

QString MainWindow::generateWeatherBriefingText()
{
    QString briefing;

    QLabel *cityNameLabel = findChild<QLabel*>("cityNameLabel");
    WeatherCard *wc = findChild<WeatherCard*>();
    QLabel *highLowLabel = findChild<QLabel*>("highLowLabel");
    QLabel *windValueLabel = findChild<QLabel*>("windValue");
    QLabel *windDirectionLabel = findChild<QLabel*>("windDirection");
    QLabel *weatherDescLabel = findChild<QLabel*>("weatherDescription");
    AlertsSection *alertsSection = findChild<AlertsSection*>();

    if (!cityNameLabel || !wc || cityNameLabel->text() == "--") {
        return "No weather data available to read.";
    }


    if (alertsSection) {
        QWidget *alertCard = alertsSection->findChild<QWidget*>("alertCardContainer");
        if (alertCard) { // An alert exists
            QLabel *alertTitle = alertCard->findChild<QLabel*>("alertTitle");
            QLabel *alertTime = alertCard->findChild<QLabel*>("alertTime");
            if (alertTitle && alertTime) {
                briefing += QString("Attention: There is an active weather alert. %1, effective %2. ")
                .arg(alertTitle->text())
                    .arg(alertTime->text());
            }
        }
    }

    briefing += QString("Here is your weather briefing for %1. ").arg(cityNameLabel->text());

    QString currentTemp = wc->tempLabel->text().replace("°", " degrees");
    QString currentCond = wc->conditionLabel->text();
    briefing += QString("It is currently %1 with %2. ").arg(currentTemp, currentCond);


    QString feelsLike, humidity;
    QList<QWidget*> statCards = findChildren<QWidget*>("statCard");
    for (QWidget* card : statCards) {
        QLabel* title = card->findChild<QLabel*>("statTitle");
        QLabel* value = card->findChild<QLabel*>("statValue");
        if (title && value) {
            if (title->text() == "Feels Like") {
                feelsLike = value->text().replace("°", " degrees");
            } else if (title->text() == "Humidity") {
                humidity = value->text();
            }
        }
    }
    if (!feelsLike.isEmpty()) {
        briefing += QString("It feels like %1. ").arg(feelsLike);
    }

    // High and Low
    if (highLowLabel) {
        QString highLowText = highLowLabel->text();
        QRegularExpression re("H: (\\d+)° L: (\\d+)°");
        QRegularExpressionMatch match = re.match(highLowText);
        if (match.hasMatch()) {
            QString high = match.captured(1);
            QString low = match.captured(2);
            briefing += QString("Today's forecast calls for a high of %1 degrees and a low of %2 degrees. ").arg(high, low);
        }
    }

    // Summary
    if (weatherDescLabel && !weatherDescLabel->text().startsWith("Loading")) {
        briefing += weatherDescLabel->text() + " ";
    }

    // Wind and Humidity
    if (windValueLabel && windDirectionLabel) {
        briefing += QString("The wind is from the %1 at %2. ").arg(windDirectionLabel->text(), windValueLabel->text());
    }
    if (!humidity.isEmpty()) {
        briefing += QString("Humidity is at %1. ").arg(humidity);
    }

    briefing += "Have a great day.";

    return briefing;
}


void MainWindow::addFavoriteCity()
{
    bool ok;
    QString city = QInputDialog::getText(this, "Add Favorite City",
                                         "Enter city name:", QLineEdit::Normal, "", &ok);
    if(ok && !city.isEmpty()) {
        CitiesSection* cs = findChild<CitiesSection*>();
        if (cs) {
            cs->addCityToList(city);
        }
        statusLabel->setText(QString("Added %1 to favorites").arg(city));
    }
}

void MainWindow::onCitySelectedFromFavorites(const QString &city)
{
    cityInput->setText(city);
    onSearchClicked();
    showSection(0);
}

void MainWindow::onCitySelected(QListWidgetItem *item)
{
    cityInput->setText(item->data(Qt::UserRole).toString());
    onSearchClicked();
    showSection(0);
}

void MainWindow::showHelp()
{
    QMessageBox::information(this, "Help",
                             "1. Search any city using the search bar or use your current location\n"
                             "2. Switch sections using the navigation menu\n"
                             "3. Click 'Weather Briefing' for a voice forecast summary.\n"
                             "4. Customize appearance and units in Settings\n"
                             "5. View detailed forecasts in the Forecasts section\n"
                             "6. Manage your favorite cities in the Cities section");
}

MainWindow::~MainWindow()
{
    delete fadeAnimation;
    delete weatherClient;
    delete ttsProvider;
}
