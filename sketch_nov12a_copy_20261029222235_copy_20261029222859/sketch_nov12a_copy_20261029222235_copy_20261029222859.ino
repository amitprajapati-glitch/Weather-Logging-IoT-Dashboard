#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// WIFI SETTINGS
// =====================================================

const char* WIFI_SSID = "AMIT's S21 FE";
const char* WIFI_PASSWORD = "";

// =====================================================
// THINGSPEAK SETTINGS
// =====================================================


const char* THINGSPEAK_API_KEY = "";

const unsigned long CHANNEL_ID = 3479207;
const unsigned long CLOUD_INTERVAL = 20000;
const unsigned long SENSOR_INTERVAL = 2000;

// =====================================================
// PIN DEFINITIONS
// =====================================================

#define DHT_PIN D5
#define DHT_TYPE DHT22

#define BUTTON_PIN D6

#define LED_PIN D7

#define BUZZER_PIN D0

// I2C:
// SDA = D2
// SCL/SCK = D1

// =====================================================
// OBJECTS
// =====================================================

DHT dht(DHT_PIN, DHT_TYPE);

Adafruit_BMP280 bmp;

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// =====================================================
// SYSTEM VARIABLES
// =====================================================

// System starts OFF
bool systemON = false;

bool bmpOK = false;

// Sensor values
float dhtTemperature = NAN;
float dhtHumidity = NAN;

float bmpTemperature = NAN;
float pressure = NAN;

// =====================================================
// BUTTON VARIABLES
// =====================================================

bool lastButtonState = HIGH;
bool currentButtonState = HIGH;

unsigned long lastDebounceTime = 0;

const unsigned long debounceDelay = 50;

// =====================================================
// SENSOR/CLOUD TIMING
// =====================================================

unsigned long lastSensorRead = 0;
unsigned long lastCloudUpdate = 0;

// =====================================================
// ALERT BUZZER VARIABLES
// =====================================================

bool alertActive = false;

bool buzzerState = false;

unsigned long lastBuzzerTime = 0;

// Time between alert beeps
const unsigned long BUZZER_INTERVAL = 1000;

// Length of alert beep
const unsigned long BEEP_DURATION = 200;

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("=================================");
  Serial.println(" WEATHER LOGGING & IoT DASHBOARD ");
  Serial.println("=================================");

  // ---------------------------------------------------
  // PIN SETUP
  // ---------------------------------------------------

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(LED_PIN, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  // Start with everything OFF
  digitalWrite(LED_PIN, LOW);

  digitalWrite(BUZZER_PIN, LOW);

  // ---------------------------------------------------
  // I2C SETUP
  // ---------------------------------------------------

  // SDA = D2
  // SCL/SCK = D1

  Wire.begin(D2, D1);

  // ---------------------------------------------------
  // DHT22
  // ---------------------------------------------------

  dht.begin();

  Serial.println("DHT22 initialized.");

  // ---------------------------------------------------
  // OLED
  // ---------------------------------------------------

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED NOT FOUND!");

  } else {

    Serial.println("OLED found at 0x3C");

    showSystemOff();
  }

  // ---------------------------------------------------
  // BMP280
  // ---------------------------------------------------

  if (bmp.begin(0x76)) {

    bmpOK = true;

    Serial.println("BMP280 found at 0x76.");

  } else {

    bmpOK = false;

    Serial.println("BMP280 NOT FOUND!");
  }

  // ---------------------------------------------------
  // INITIAL SYSTEM STATE
  // ---------------------------------------------------

  Serial.println();
  Serial.println("SYSTEM STARTED IN OFF MODE.");
  Serial.println("Press the button to turn ON.");

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

// =====================================================
// STARTUP / OFF SCREEN
// =====================================================

void showSystemOff() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(10, 8);

  display.println("SYSTEM");

  display.setCursor(28, 35);

  display.println("OFF");

  display.display();
}

// =====================================================
// STARTING SCREEN
// =====================================================

void showStartingScreen() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println("Weather Logging");

  display.setCursor(0, 15);

  display.println("IoT Dashboard");

  display.setCursor(0, 30);

  display.println("System Starting...");

  display.display();
}

// =====================================================
// SHORT BEEP FOR ON/OFF
// =====================================================

void startupShutdownBeep() {

  digitalWrite(BUZZER_PIN, HIGH);

  delay(200);

  digitalWrite(BUZZER_PIN, LOW);
}

// =====================================================
// WIFI CONNECTION
// =====================================================

void connectWiFi() {

  Serial.println();

  Serial.print("Connecting to Wi-Fi: ");

  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 30) {

    delay(500);

    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("Wi-Fi connected!");

    Serial.print("IP Address: ");

    Serial.println(WiFi.localIP());

    digitalWrite(LED_PIN, HIGH);

  } else {

    Serial.println("Wi-Fi connection failed.");

    digitalWrite(LED_PIN, LOW);
  }
}

// =====================================================
// TURN SYSTEM ON
// =====================================================

void turnSystemON() {

  systemON = true;

  Serial.println();
  Serial.println("=================================");
  Serial.println("SYSTEM TURNED ON");
  Serial.println("=================================");

  // ---------------------------------------------------
  // ON BEEP
  // ---------------------------------------------------

  startupShutdownBeep();

  // ---------------------------------------------------
  // OLED
  // ---------------------------------------------------

  showStartingScreen();

  // ---------------------------------------------------
  // WIFI
  // ---------------------------------------------------

  connectWiFi();

  // ---------------------------------------------------
  // RESET TIMERS
  // ---------------------------------------------------

  lastSensorRead = 0;

  lastCloudUpdate = millis();

  // Reset alert buzzer
  alertActive = false;

  buzzerState = false;

  lastBuzzerTime = millis();

  delay(1000);
}

// =====================================================
// TURN SYSTEM OFF
// =====================================================

void turnSystemOFF() {

  Serial.println();
  Serial.println("=================================");
  Serial.println("SYSTEM TURNED OFF");
  Serial.println("=================================");

  // ---------------------------------------------------
  // OFF BEEP
  // ---------------------------------------------------

  startupShutdownBeep();

  // Now set system OFF
  systemON = false;

  // ---------------------------------------------------
  // TURN OFF OUTPUTS
  // ---------------------------------------------------

  digitalWrite(LED_PIN, LOW);

  digitalWrite(BUZZER_PIN, LOW);

  buzzerState = false;

  alertActive = false;

  // ---------------------------------------------------
  // DISCONNECT WIFI
  // ---------------------------------------------------

  WiFi.disconnect();

  // ---------------------------------------------------
  // CLEAR SENSOR VALUES
  // ---------------------------------------------------

  dhtTemperature = NAN;

  dhtHumidity = NAN;

  bmpTemperature = NAN;

  pressure = NAN;

  // ---------------------------------------------------
  // OLED OFF
  // ---------------------------------------------------

  showSystemOff();
}

// =====================================================
// BUTTON CHECK
// =====================================================

void checkButton() {

  bool reading = digitalRead(BUTTON_PIN);

  // Detect button state change
  if (reading != lastButtonState) {

    lastDebounceTime = millis();
  }

  // Debounce
  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != currentButtonState) {

      currentButtonState = reading;

      // Button pressed
      if (currentButtonState == LOW) {

        if (systemON) {

          turnSystemOFF();

        } else {

          turnSystemON();
        }
      }
    }
  }

  lastButtonState = reading;
}

// =====================================================
// READ SENSORS
// =====================================================

void readSensors() {

  // ---------------------------------------------------
  // DHT22
  // ---------------------------------------------------

  float newDHTTemperature = dht.readTemperature();

  float newDHTHumidity = dht.readHumidity();

  if (!isnan(newDHTTemperature)) {

    dhtTemperature = newDHTTemperature;
  }

  if (!isnan(newDHTHumidity)) {

    dhtHumidity = newDHTHumidity;
  }

  // ---------------------------------------------------
  // BMP280
  // ---------------------------------------------------

  if (bmpOK) {

    bmpTemperature = bmp.readTemperature();

    // Convert Pa to hPa

    pressure = bmp.readPressure() / 100.0;
  }

  // ---------------------------------------------------
  // SERIAL MONITOR
  // ---------------------------------------------------

  Serial.println();

  Serial.println("---------------------------------");

  Serial.print("DHT22 Temperature: ");

  if (isnan(dhtTemperature)) {

    Serial.println("ERROR");

  } else {

    Serial.print(dhtTemperature, 2);

    Serial.println(" °C");
  }

  Serial.print("DHT22 Humidity: ");

  if (isnan(dhtHumidity)) {

    Serial.println("ERROR");

  } else {

    Serial.print(dhtHumidity, 2);

    Serial.println(" %");
  }

  Serial.print("BMP280 Temperature: ");

  if (isnan(bmpTemperature)) {

    Serial.println("ERROR");

  } else {

    Serial.print(bmpTemperature, 2);

    Serial.println(" °C");
  }

  Serial.print("Atmospheric Pressure: ");

  if (isnan(pressure)) {

    Serial.println("ERROR");

  } else {

    Serial.print(pressure, 2);

    Serial.println(" hPa");
  }

  Serial.println("---------------------------------");

  // Update OLED
  updateOLED();

  // Check alert
  checkAlert();
}

// =====================================================
// OLED DISPLAY
// =====================================================

void updateOLED() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  if (!systemON) {

    showSystemOff();

    return;
  }

  display.setTextSize(1);

  // DHT Temperature

  display.setCursor(0, 0);

  display.print("DHT T: ");

  if (isnan(dhtTemperature)) {

    display.print("ERR");

  } else {

    display.print(dhtTemperature, 1);

    display.print(" C");
  }

  // DHT Humidity

  display.setCursor(0, 12);

  display.print("DHT H: ");

  if (isnan(dhtHumidity)) {

    display.print("ERR");

  } else {

    display.print(dhtHumidity, 1);

    display.print(" %");
  }

  // BMP Temperature

  display.setCursor(0, 24);

  display.print("BMP T: ");

  if (!bmpOK || isnan(bmpTemperature)) {

    display.print("ERR");

  } else {

    display.print(bmpTemperature, 1);

    display.print(" C");
  }

  // Pressure

  display.setCursor(0, 36);

  display.print("Pressure: ");

  if (!bmpOK || isnan(pressure)) {

    display.print("ERR");

  } else {

    display.print(pressure, 1);

    display.print(" hPa");
  }

  // Wi-Fi

  display.setCursor(0, 50);

  if (WiFi.status() == WL_CONNECTED) {

    display.print("WiFi: Connected");

  } else {

    display.print("WiFi: Offline");
  }

  display.display();
}

// =====================================================
// WEATHER ALERT
// =====================================================

void checkAlert() {

  if (!systemON) {

    alertActive = false;

    digitalWrite(BUZZER_PIN, LOW);

    buzzerState = false;

    return;
  }

  alertActive = false;

  // Temperature threshold
  if (!isnan(dhtTemperature) && dhtTemperature > 35.0) {

    alertActive = true;
  }

  // Humidity threshold
  if (!isnan(dhtHumidity) && dhtHumidity > 85.0) {

    alertActive = true;
  }

  // If no alert
  if (!alertActive) {

    digitalWrite(BUZZER_PIN, LOW);

    buzzerState = false;

    return;
  }

  // Alert is active
  beepAlert();
}

// =====================================================
// REPEATED ALERT BEEP
// =====================================================

void beepAlert() {

  unsigned long currentTime = millis();

  // Start beep
  if (!buzzerState) {

    if (currentTime - lastBuzzerTime >= BUZZER_INTERVAL) {

      digitalWrite(BUZZER_PIN, HIGH);

      buzzerState = true;

      lastBuzzerTime = currentTime;
    }
  }

  // Stop beep after 200 ms
  else {

    if (currentTime - lastBuzzerTime >= BEEP_DURATION) {

      digitalWrite(BUZZER_PIN, LOW);

      buzzerState = false;

      lastBuzzerTime = currentTime;
    }
  }
}

// =====================================================
// THINGSPEAK UPDATE
// =====================================================

void sendToThingSpeak() {

  if (!systemON) {

    Serial.println("System OFF - cloud update skipped.");

    return;
  }

  // ---------------------------------------------------
  // WIFI CHECK
  // ---------------------------------------------------

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Wi-Fi disconnected.");

    connectWiFi();

    if (WiFi.status() != WL_CONNECTED) {

      Serial.println("Cloud update failed - no Wi-Fi.");

      return;
    }
  }

  // ---------------------------------------------------
  // SENSOR CHECK
  // ---------------------------------------------------

  if (isnan(dhtTemperature) ||
      isnan(dhtHumidity) ||
      isnan(bmpTemperature) ||
      isnan(pressure)) {

    Serial.println("Invalid sensor data.");

    Serial.println("ThingSpeak update skipped.");

    return;
  }

  // ---------------------------------------------------
  // CREATE URL
  // ---------------------------------------------------

  String url = "https://api.thingspeak.com/update?api_key=";

  url += THINGSPEAK_API_KEY;

  // Field 1 - DHT22 Temperature

  url += "&field1=";

  url += String(dhtTemperature, 2);

  // Field 2 - DHT22 Humidity

  url += "&field2=";

  url += String(dhtHumidity, 2);

  // Field 3 - BMP280 Temperature

  url += "&field3=";

  url += String(bmpTemperature, 2);

  // Field 4 - Pressure

  url += "&field4=";

  url += String(pressure, 2);

  // Field 5 - System Status

  url += "&field5=1";

  // ---------------------------------------------------
  // SEND TO THINGSPEAK
  // ---------------------------------------------------

  Serial.println();

  Serial.println("Sending data to ThingSpeak...");

  WiFiClientSecure client;

  // For prototype/testing
  client.setInsecure();

  HTTPClient http;

  if (http.begin(client, url)) {

    int httpCode = http.GET();

    if (httpCode > 0) {

      Serial.print("HTTP Response: ");

      Serial.println(httpCode);

      String response = http.getString();

      Serial.print("ThingSpeak response: ");

      Serial.println(response);

      if (response != "0") {

        Serial.println("ThingSpeak update successful!");

      } else {

        Serial.println("ThingSpeak update failed.");
      }

    } else {

      Serial.print("HTTP error: ");

      Serial.println(http.errorToString(httpCode));
    }

    http.end();

  } else {

    Serial.println("Unable to connect to ThingSpeak.");
  }
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  // Button always works
  checkButton();

  // ---------------------------------------------------
  // SYSTEM ON
  // ---------------------------------------------------

  if (systemON) {

    // Sensor reading every 2 seconds

    if (millis() - lastSensorRead >= SENSOR_INTERVAL) {

      lastSensorRead = millis();

      readSensors();
    }

    // ThingSpeak update every 20 seconds

    if (millis() - lastCloudUpdate >= CLOUD_INTERVAL) {

      lastCloudUpdate = millis();

      sendToThingSpeak();
    }

    // LED indicates Wi-Fi connection

    if (WiFi.status() == WL_CONNECTED) {

      digitalWrite(LED_PIN, HIGH);

    } else {

      digitalWrite(LED_PIN, LOW);
    }
  }

  // ---------------------------------------------------
  // SYSTEM OFF
  // ---------------------------------------------------

  else {

    digitalWrite(LED_PIN, LOW);

    digitalWrite(BUZZER_PIN, LOW);

    buzzerState = false;

    alertActive = false;
  }
}
