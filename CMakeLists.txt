cmake_minimum_required(VERSION 3.19)
project(zephyr LANGUAGES CXX)

# Find all necessary Qt6 components.
# FIX: Added the 'Network' module for API calls.
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui Multimedia Svg Charts SvgWidgets Positioning Location TextToSpeech Network)

qt_standard_project_setup()

# List all your source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/weathercard.cpp
    src/forecastsection.cpp
    src/citiessection.cpp
    src/mapssection.cpp
    src/forecastssection.cpp
    src/alertssection.cpp
    src/settingsdialog.cpp
    src/settings/SettingsManager.cpp
)

# List all your header files
set(HEADERS
    src/mainwindow.h
    src/weathercard.h
    src/forecastsection.h
    src/citiessection.h
    src/mapssection.h
    src/forecastssection.h
    src/alertssection.h
    src/settingsdialog.h
    src/settings/SettingsManager.h
    src/interfaces/IDataAnalyzer.h
    src/interfaces/IHeatMapGenerator.h
    src/interfaces/ITTSProvider.h
    src/interfaces/IWeatherClient.h
    src/apikey.h
)

# Add executable
qt_add_executable(zephyr
    ${SOURCES}
    ${HEADERS}
)

# Add resources
qt_add_resources(zephyr "resources"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    FILES
        resources/resources.qrc
)

# Include directories
target_include_directories(zephyr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/src/settings
)

# Link all necessary libraries
# FIX: Added Qt6::Network to link against.
target_link_libraries(zephyr PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Charts
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::Positioning
    Qt6::Location
    Qt6::TextToSpeech
    Qt6::Network
)

# Enable C++17
target_compile_features(zephyr PRIVATE cxx_std_17)

# Installation target (optional)
include(GNUInstallDirs)
install(TARGETS zephyr BUNDLE DESTINATION .)
