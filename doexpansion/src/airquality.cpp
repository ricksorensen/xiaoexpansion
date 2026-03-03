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
bool checkAuth();

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

void reconnectMQTT() {
  if (mqttClient.connected())
    return;
  Serial.print("Attempting MQTT connection...");
  if (mqttClient.connect("ESP32_C6_AirQuality", mqtt_user, mqtt_password)) {
    Serial.println("connected");

    // Publish Home Assistant auto-discovery payloads
    const char *base_topic = "homeassistant/sensor/airquality_c6";
    const char *state_topic = "airquality/state";

    struct SensorDef {
      const char *id;
      const char *name;
      const char *unit;
      const char *device_class;
      const char *val_tpl;
    };

    SensorDef sensors[] = {
        {"temp", "Temperature",
         "\xC2\xB0"
         "C",
         "temperature", "{{ value_json.temperature | round(1) }}"},
        {"hum", "Humidity", "%", "humidity",
         "{{ value_json.humidity | round(1) }}"},
        {"pm25", "PM2.5", "\xC2\xB5g/m\xC2\xB3", "pm25",
         "{{ value_json.pm2_5 }}"},
        {"voc", "VOC Raw", "", "aqi", "{{ value_json.voc_raw }}"},
        {"nox", "NOx Raw", "", "aqi", "{{ value_json.nox_raw }}"},
        {"pres", "Pressure", "hPa", "atmospheric_pressure",
         "{{ value_json.pressure | round(1) }}"},
        {"spatemp", "SPA Temperature",
         "\xC2\xB0"
         "C",
         "temperature", "{{ value_json.spa_temp | round(1) }}"}};

    for (int i = 0; i < 7; i++) {
      StaticJsonDocument<512> doc;
      doc["name"] = String("AirQuality ") + sensors[i].name;
      doc["state_topic"] = state_topic;
      if (strlen(sensors[i].unit) > 0)
        doc["unit_of_measurement"] = sensors[i].unit;
      doc["device_class"] = sensors[i].device_class;
      doc["value_template"] = sensors[i].val_tpl;
      doc["unique_id"] = String("airquality_c6_") + sensors[i].id;

      JsonObject device = doc.createNestedObject("device");
      JsonArray identifiers = device.createNestedArray("identifiers");
      identifiers.add("airquality_c6_device");
      device["name"] = "ESP32-C6 Air Quality Monitor";
      device["model"] = "XIAO ESP32-C6 Expansion";
      device["manufacturer"] = "Custom";

      String payload;
      serializeJson(doc, payload);
      String topic = String(base_topic) + "_" + sensors[i].id + "/config";
      mqttClient.publish(topic.c_str(), payload.c_str(), true); // Retained
    }
  } else {
    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" try again in 5 seconds");
  }
}

void loop() {
  if (!mqttClient.connected()) {
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = millis();
      reconnectMQTT();
    }
  }
  mqttClient.loop();

  server.handleClient();

  if (millis() - lastReadTime >= readInterval) {
    lastReadTime = millis();
    readSensors();
    updateOLED();

    if (mqttClient.connected()) {
      StaticJsonDocument<200> doc;
      doc["temperature"] = tempC;
      doc["humidity"] = humidity;
      doc["pm2_5"] = pm25;
      doc["voc_raw"] = srawVoc;
      doc["nox_raw"] = srawNox;
      doc["pressure"] = pressure_hPa;
      doc["spa_temp"] = spa06_tempC;
      String payload;
      serializeJson(doc, payload);
      mqttClient.publish("airquality/state", payload.c_str());
    }
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

bool checkAuth() {
  // Define credentials inside - in a real app, store these securely or make
  // configurable
  const char *www_username = "admin";
  const char *www_password = "password123";
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

void handleRoot() {
  if (!checkAuth())
    return;

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Air Quality Monitor</title>
<style>
  :root {
    --bg: #1a1a2e;
    --panel: #16213e;
    --text: #e94560;
    --highlight: #0f3460;
  }
  body {
    background: var(--bg); color: #fff; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    margin: 0; padding: 20px;
  }
  .container { max-width: 800px; margin: auto; }
  .tabs { display: flex; gap: 10px; margin-bottom: 20px; }
  .tab { 
    padding: 10px 20px; background: var(--panel); cursor: pointer;
    border-radius: 5px; transition: 0.3s;
  }
  .tab.active { background: var(--highlight); color: #fff; }
  .panel { display: none; background: var(--panel); padding: 20px; border-radius: 8px; }
  .panel.active { display: block; }
  .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; }
  .card { 
    background: #0f3460; padding: 15px; border-radius: 8px; text-align: center;
    box-shadow: 0 4px 6px rgba(0,0,0,0.3);
  }
  .card h3 { margin: 0 0 10px 0; color: #a2a2bd; font-size: 14px; }
  .card .val { font-size: 24px; font-weight: bold; color: var(--text); }
  input, button { padding: 10px; margin-top: 10px; border-radius: 5px; border: none; }
  button { background: var(--text); color: white; cursor: pointer; }
</style>
</head>
<body>
<div class="container">
  <h2>Air Quality Monitor</h2>
  <div class="tabs">
    <div class="tab active" onclick="showTab('dash')">Dashboard</div>
    <div class="tab" onclick="showTab('settings')">Settings</div>
  </div>

  <div id="dash" class="panel active">
    <div class="grid">
      <div class="card"><h3>Temperature (DHT)</h3><div class="val" id="t-dht">-- &deg;C</div></div>
      <div class="card"><h3>Humidity (DHT)</h3><div class="val" id="h-dht">-- %</div></div>
      <div class="card"><h3>Temperature (SPA)</h3><div class="val" id="t-spa">-- &deg;C</div></div>
      <div class="card"><h3>Pressure</h3><div class="val" id="p-spa">-- hPa</div></div>
      <div class="card"><h3>PM 2.5</h3><div class="val" id="v-pm">-- &micro;g/m&sup3;</div></div>
      <div class="card"><h3>VOC (Raw)</h3><div class="val" id="v-voc">--</div></div>
      <div class="card"><h3>NOx (Raw)</h3><div class="val" id="v-nox">--</div></div>
    </div>
  </div>

  <div id="settings" class="panel">
    <h3>MQTT Configuration</h3>
    <p>Update broker details securely (Coming Soon).</p>
    <input type="text" placeholder="Broker IP"><br>
    <button>Save configuration</button>
  </div>
</div>

<script>
  function showTab(id) {
    document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
    document.getElementById(id).classList.add('active');
    event.target.classList.add('active');
  }

  async function updateData() {
    try {
      const res = await fetch('/api/data');
      if (res.status === 401) return; // Auth required, ignore fetch if kicked out
      const data = await res.json();
      document.getElementById('t-dht').innerText = data.temperature.toFixed(1) + ' °C';
      document.getElementById('h-dht').innerText = data.humidity.toFixed(1) + ' %';
      document.getElementById('t-spa').innerText = data.spa_temp.toFixed(1) + ' °C';
      document.getElementById('p-spa').innerText = data.pressure.toFixed(1) + ' hPa';
      document.getElementById('v-pm').innerText = data.pm2_5 + ' µg/m³';
      document.getElementById('v-voc').innerText = data.voc_raw;
      document.getElementById('v-nox').innerText = data.nox_raw;
    } catch(e) { console.error('Fetch error:', e); }
  }
  
  setInterval(updateData, 5000);
  updateData();
</script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void handleApiData() {
  if (!checkAuth())
    return; // Secure API endpoint as well

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
