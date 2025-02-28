#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>

// CONFIGURATION OPTIONS START
const char *ssid = "REPLACE_WITH_YOUR_SSID";
const char *password = "REPLACE_WITH_YOUR_PASSWORD";
String apiKey = "REPLACE_WITH_YOUR_OPENWEATHERMAP_API";
String city = "Bucharest"; // City
String countryCode = "RO"; // ISO country code

const long utcOffsetInSeconds = 7200;  // example for UTC+2 (2h=7200s). Change timezone if needed.


// CONFIGURATION OPTIONS END

String weatherNowURL = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + apiKey + "&units=metric";
String weatherForecastURL = "http://api.openweathermap.org/data/2.5/forecast?q=" + city + "," + countryCode + "&cnt=9&appid=" + apiKey + "&units=metric";

char daysOfTheWeek[7][5] = { "Sun ", "Mon ", "Tue ", "Wed ", "Thu ", "Fri ", "Sat " };
String jsonBuffer;
JSONVar weatherObject;
String jsonForecastBuffer;
JSONVar weatherForecastObject;
String formattedDate, dateText;

unsigned long lastTime = 0;
unsigned long lastTime_secondScreen = 0;
unsigned long timerDelay = 120000;
unsigned long lastTime_backlight = 0;
bool backlightStatus = false;
bool manualBacklight = false;
int current_screen = 0;
bool firstTime = true;
bool firstTime_secondScreen = true;

#define LED_PIN D5
#define BTN_PIN D6
#define BACKLIGHT_PIN D7
#define SENSOR_PIN A0

Adafruit_PCD8544 display = Adafruit_PCD8544(D4, D3, D2, D1, D0);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);
WiFiClient client;

String httpGETRequest(const char *serverName) {
  WiFiClient client;
  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void updateWeather() {
  if (WiFi.status() == WL_CONNECTED) {
    if (firstTime || ((millis() - lastTime) > timerDelay)) {
      jsonBuffer = httpGETRequest(weatherNowURL.c_str());
      weatherObject = JSON.parse(jsonBuffer);

      firstTime = false;
      lastTime = millis();
    }

    display.setCursor(0, 27);
    display.print(int(weatherObject["main"]["temp"]));

    display.print(" C");
    display.print(" ");
    display.print(String(weatherObject["weather"][0]["main"]));

    display.setCursor(0, 36);
    display.print("Wind: ");
    display.print(weatherObject["wind"]["speed"]);
    display.print(" ");
    display.print("m/s");
  } else {
    display.println("Waiting for");
    display.print("network...");
  }
}

void displayMainScreen() {
  display.clearDisplay();
  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.setCursor(0, 0);

  // get and display time
  if (timeClient.getHours() < 10)
    display.print(0);
  display.print(timeClient.getHours());
  display.print(":");
  if (timeClient.getMinutes() < 10)
    display.print(0);
  display.print(timeClient.getMinutes());
  display.setTextSize(1);
  if (timeClient.getSeconds() < 10)
    display.print(0);
  display.print(timeClient.getSeconds());

  display.setCursor(0, 18);
  display.setTextSize(1);
  display.print(daysOfTheWeek[timeClient.getDay()]);

  formattedDate = timeClient.getFormattedDate();
  dateText = formattedDate.substring(0, formattedDate.indexOf("T"));
  display.print(dateText);

  // get and display weather
  updateWeather();

  display.display();
}

void displaySecondaryScreen() {
  display.clearDisplay();
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(0, 0);

  if (WiFi.status() == WL_CONNECTED) {
    if (firstTime_secondScreen || ((millis() - lastTime_secondScreen) > timerDelay)) {
      jsonForecastBuffer = httpGETRequest(weatherForecastURL.c_str());
      weatherForecastObject = JSON.parse(jsonForecastBuffer);

      firstTime_secondScreen = false;
      lastTime_secondScreen = millis();
    }

    display.println(String(weatherForecastObject["list"][8]["dt_txt"]));

    display.print(int(weatherForecastObject["list"][8]["main"]["temp"]));
    display.print(" C ");
    display.println(String(weatherForecastObject["list"][8]["weather"][0]["main"]));

    display.print("Humidity: ");
    display.print(int(weatherForecastObject["list"][8]["main"]["humidity"]));
    display.println("%");

    display.print("Wind: ");
    display.print(weatherForecastObject["list"][8]["wind"]["speed"]);
    display.println(" m/s");
  } else {
    display.println("Waiting for");
    display.print("network...");
  }

  display.display();
}

void screenSelector() {
  if (backlightStatus && (digitalRead(BTN_PIN) == LOW)) {
    if (current_screen == 0)
      current_screen = 1;
    else
      current_screen = 0;
    delay(500);
  }

  if (current_screen == 0)
    displayMainScreen();
  if (current_screen == 1)
    displaySecondaryScreen();
}

void backlightControl() {
  if (analogRead(SENSOR_PIN) > 600) {
    digitalWrite(BACKLIGHT_PIN, HIGH);
    backlightStatus = true;
  } else if (!manualBacklight) {
    digitalWrite(BACKLIGHT_PIN, LOW);
    backlightStatus = false;
  }

  if ((digitalRead(BTN_PIN) == LOW) && (analogRead(SENSOR_PIN) <= 600)) {
    lastTime_backlight = millis();
    digitalWrite(BACKLIGHT_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    backlightStatus = true;
    manualBacklight = true;
    delay(500);
  }
  if (((millis() - lastTime_backlight) > 15000) && (analogRead(SENSOR_PIN) <= 600)) {
    digitalWrite(BACKLIGHT_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    backlightStatus = false;
    manualBacklight = false;
  }
}

void timekeeping() {
  timeClient.update();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BACKLIGHT_PIN, OUTPUT);

  display.begin();
  display.setContrast(50);

  WiFi.begin(ssid, password);
  timeClient.begin();
}

void loop() {
  timekeeping();
  backlightControl();
  screenSelector();
  backlightControl();
  delay(500);
}
