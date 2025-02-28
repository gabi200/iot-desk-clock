# IoT Desk Clock
IoT Desk Clock with automatic NTP time sync and weather display

The ESP8266 code is built on the Arduino framework. 

## Features
- automatic Network Time Protocol (NTP) based sync
- OpenWeatherMap integration for current weather conditions
- option to display weather for the next day
- automatic display backlight switching based on ambient light

## Software Dependencies
- Adafruit_GFX
- Adafruit_PCD8544
- ESP8266WiFi
- WiFiUdp, WiFiClient
- Arduino_JSON
- ESP8266HTTPClient
- NTPClient

## Hardware Components
- PCD8544-based LCD display
- Push button
- Photoresistor
- ESP8266 devboard
- 5V USB power supply

**Note:** Please replace the corresponding placeholder strings with your 2.4GHz Wi-Fi SSID, password and OpenWeatherMap API key. Change the city, ISO country code and UTC offset for your location.
