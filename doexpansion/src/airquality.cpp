#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <Wire.h>

// --- Sensor & Display Libraries ---
#include "DHT.h"
#include "SPL07-003.h"
#include "Seeed_HM330X.h"
#include "TCA9548.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SensirionI2CSgp41.h>

// --- Pin Definitions ---
// On the XIAO ESP32-C6, A0/D0 is typically GPIO 2, but Seeed core uses D0-D3
// names
#define DHTPIN                                                                 \
  D0 // Using D0 as it maps to the A0 pin on the expansion board shield
#define DHTTYPE DHT11

// --- Hardware Objects ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

TCA9548 multiplexer(0x70); // Default Pa.HUB address

DHT dht(DHTPIN, DHTTYPE);
SensirionI2CSgp41 sgp41;
HM330X hm330x;
SPL07_003 spa06;

WebServer server(80);

// --- MQTT Configuration ---
// TO DO: Replace with your actual Home Assistant IP and credentials
const char *mqtt_server = "192.168.1.100";
const int mqtt_port = 1883;
const char *mqtt_user = "your_mqtt_user";
const char *mqtt_password = "your_mqtt_pass";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// --- Global Variables for Data ---
float tempC = 0.0;
float humidity = 0.0;
uint16_t srawVoc = 0;
uint16_t srawNox = 0;
uint16_t pm25 = 0;
float pressure_hPa = 0.0;
float spa06_tempC = 0.0;

unsigned long lastReadTime = 0;
const long readInterval = 5000; // Read sensors every 5 seconds

// --- Function Declarations ---
void setupDisplay();
void setupSensors();
void readSensors();
void updateOLED();
void reconnectMQTT();
void setupWebServer();
void handleRoot();
void handleApiData();

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nStarting Air Quality Monitor ESP32-C6...");

  // 1. Initialize Display First (to show connection status)
  Wire.begin();
  setupDisplay();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Booting...");
  display.println("Starting WiFiSetup");
  display.display();

  // 2. Initialize WiFi via WiFiManager
  WiFiManager wm;
  // wm.resetSettings(); // Uncomment to wipe saved WiFi credentials for testing
  bool res =
      wm.autoConnect("AirQuality_Setup", "password123"); // AP Name and Password
  if (!res) {
    Serial.println("Failed to connect to WiFi and hit timeout");
    display.println("WiFi Failed. Restarting.");
    display.display();
    delay(3000);
    ESP.restart();
  }

  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // 3. Initialize Sensors
  setupSensors();

  // 4. Initialize MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);

  // 5. Initialize Web Server
  setupWebServer();
  server.begin();
  Serial.println("HTTP Server Started");
}

void loop() {
  // Ensure MQTT connection
  // if (!mqttClient.connected()) {
  //     reconnectMQTT();
  // }
  // mqttClient.loop();

  server.handleClient();

  // Non-blocking delay for reading sensors
  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();
    readSensors();
    updateOLED();

    // TODO: Publish data to MQTT
  }
}

// --- Helper Functions ---

void setupDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
}

void setupSensors() {
  Serial.println("Initializing Sensors...");

  // Init Multiplexer
  if (!multiplexer.begin()) {
    Serial.println("TCA9548A Multiplexer not found. Check wiring.");
  } else {
    Serial.println("TCA9548A Multiplexer found.");
  }

  // Init DHT11 (Direct GPIO)
  dht.begin();

  // Init HM3301 (Root I2C Bus)
  if (hm330x.init()) {
    Serial.println("HM330X init failed!");
  } else {
    Serial.println("HM330X initialized.");
  }

  // Init SPA06 (Pa.HUB Channel 0)
  multiplexer.selectChannel(0);
  delay(50);
  spa06.begin();
  Serial.println("SPA06 initialized on Channel 0.");

  // Init SGP41 (Pa.HUB Channel 1)
  multiplexer.selectChannel(1);
  delay(50);
  sgp41.begin(Wire);
  uint16_t error;
  char errorMessage[256];
  uint16_t serialNumber[3];
  error = sgp41.getSerialNumber(serialNumber);
  if (error) {
    Serial.print("SGP41 getSerialNumber error on Channel 1: ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    Serial.println("SGP41 initialized on Channel 1.");
  }
}

void readSensors() {
  // Read DHT11
  tempC = dht.readTemperature();
  humidity = dht.readHumidity();
  if (isnan(tempC) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  }

  // Read HM3301 (Root bus)
  uint8_t buf[30];
  if (hm330x.read_sensor_value(buf, 29)) {
    Serial.println("HM330X read failed");
  } else {
    pm25 = (uint16_t)buf[6] << 8 | buf[7];
  }

  // Read SPA06 (Channel 0)
  multiplexer.selectChannel(0);
  pressure_hPa = spa06.readPressure() / 100.0; // Pa to hPa
  spa06_tempC = spa06.readTemperature();

  // Read SGP41 (Channel 1)
  multiplexer.selectChannel(1);
  // Use DHT11 data to compensate SGP41 (requires precise rh/t ticks
  // calculation, using defaults for now to ensure it runs)
  uint16_t defaultRh = 0x8000;
  uint16_t defaultT = 0x6666;
  uint16_t error =
      sgp41.measureRawSignals(defaultRh, defaultT, srawVoc, srawNox);
  if (error) {
    Serial.println("Error reading SGP41");
  }

  Serial.printf("Temp: %.1fC (SPA: %.1fC), Hum: %.1f%%, PM2.5: %d, VOC: %d, "
                "NOx: %d, Prs: %.1f hPa\n",
                tempC, spa06_tempC, humidity, pm25, srawVoc, srawNox,
                pressure_hPa);
}

void updateOLED() {
  display.clearDisplay();
  display.setCursor(0, 0);

  const int lineHeight = 8;
  int currentY = 0;

  display.println("IP:" + WiFi.localIP().toString());
  currentY += lineHeight;

  String lines[] = {
      "Temp: " + String(tempC, 1) + " C", "Hum: " + String(humidity, 1) + " %",
      "PM2.5: " + String(pm25) + " ug/m3", "VOC: " + String(srawVoc),
      "Pres: " + String(pressure_hPa, 1) + "hPa"};

  int numLines = sizeof(lines) / sizeof(lines[0]);

  for (int i = 0; i < numLines; i++) {
    if ((currentY + lineHeight) <= SCREEN_HEIGHT) {
      display.println(lines[i]);
      currentY += lineHeight;
    } else {
      display.fillRect(0, SCREEN_HEIGHT - lineHeight, SCREEN_WIDTH, lineHeight,
                       SSD1306_BLACK);
      display.setCursor(0, SCREEN_HEIGHT - lineHeight);
      display.println("WARN: Data Overflow");
      break;
    }
  }
  display.display();
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/data", HTTP_GET, handleApiData);
}

void handleRoot() {
  server.send(200, "text/plain", "Air Quality Monitor UI - Under Construction");
}

void handleApiData() {
  StaticJsonDocument<200> doc;
  doc["temperature"] = tempC;
  doc["humidity"] = humidity;
  doc["pm2_5"] = pm25;
  doc["voc_raw"] = srawVoc;
  doc["nox_raw"] = srawNox;
  doc["pressure"] = pressure_hPa;
  doc["spa_temp"] = spa06_tempC;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}
