# Zephyr Weather Station

<p align="center">
  <img src="resources/icons/zephyr_logo.svg" alt="Zephyr Logo" width="200"/>
</p>

<p align="center">
  <strong>A modern, cross-platform desktop weather application built with C++ and Qt.</strong>
</p>

<p align="center">
  <a href="#key-features">Key Features</a> •
  <a href="#technology-stack">Technology Stack</a> •
  <a href="#getting-started">Getting Started</a> •
  <a href="#usage">Usage</a> •
  <a href="#contributing">Contributing</a> •
  <a href="#license">License</a>
</p>

---

Zephyr is a feature-rich and user-friendly weather application that provides real-time weather information, forecasts, and maps. It is designed to be a reliable and comprehensive tool for anyone who needs to stay informed about the weather.

## Key Features

*   **Real-time Weather:** Get up-to-the-minute weather conditions for any location worldwide.
*   **Detailed Forecasts:** View detailed daily and hourly forecasts, including temperature, precipitation, wind, and more.
*   **Interactive Maps:** Explore weather patterns with interactive maps for temperature, precipitation, and cloud cover.
*   **Customizable Locations:** Save and manage a list of your favorite cities for quick access.
*   **Weather Alerts:** Receive notifications for severe weather alerts in your area.
*   **Light and Dark Themes:** Choose between light and dark themes for a personalized user experience.
*   **Text-to-Speech:** Listen to the weather forecast with the integrated text-to-speech feature.

## Technology Stack

*   **C++17:** For a high-performance, cross-platform backend.
*   **Qt 6.5+:** For the graphical user interface and application framework.
*   **CMake:** For cross-platform build automation.

## Getting Started

### Prerequisites

*   **Qt 6.5 or higher:** You can download Qt from the [official website](https://www.qt.io/download).
*   **A C++17 compliant compiler:** (e.g., GCC, Clang, MSVC).
*   **CMake 3.19 or higher:** You can download CMake from the [official website](https://cmake.org/download/).

### Building

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-username/zephyr.git
    cd zephyr
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Run CMake:**
    ```bash
    cmake ..
    ```

4.  **Build the project:**
    ```bash
    cmake --build .
    ```

### Configuration

Zephyr requires an API key from a weather data provider (openweather api (paid tier)) to fetch weather information. The application expects the API key to be defined in `src/apikey.h`.

1.  Create a file named `apikey.h` in the `src` directory.
2.  Add the following content to the file:

    ```cpp
    #pragma once

    #define API_KEY "YOUR_API_KEY"
    ```

3.  Replace `"YOUR_API_KEY"` with your actual API key.

## Usage

Once you have built and configured the application, you can run the executable from the build directory. The main window will display the current weather for your default location. You can use the navigation bar to switch between the different sections of the application:

*   **Forecast:** View detailed weather forecasts.
*   **Cities:** Manage your list of saved locations.
*   **Maps:** Explore interactive weather maps.
*   **Alerts:** View any active weather alerts.

---
## Issue
Hardcoded svg and png paths (Qt issue)
---

## Contributing

Contributions are welcome! If you would like to contribute to Zephyr, please follow these steps:

1.  Fork the repository.
2.  Create a new branch for your feature or bug fix.
3.  Make your changes and commit them with descriptive messages.
4.  Push your changes to your fork.
5.  Create a pull request to the main repository.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.
