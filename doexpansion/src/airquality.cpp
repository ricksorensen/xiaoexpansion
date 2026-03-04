#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Preferences.h>
#include <Wire.h>

// --- Sensor & Display Libraries ---
#include "DHT.h"
#include "SPL07-003.h"
#include "Seeed_HM330X.h"
#include "TCA9548.h"
#include <U8x8lib.h>
#include <SensirionI2CSgp41.h>

// --- Pin Definitions ---
// On the XIAO ESP32-C6, A0/D0 is typically GPIO 2, but Seeed core uses D0-D3
// names
#define DHTPIN D0 // Using D0 as it maps to the A0 pin on the expansion board shield
#define DHTTYPE DHT11

// --- Hardware Objects ---
#if defined(ARDUINO_XIAO_ESP32C3) || defined(ARDUINO_XIAO_ESP32C6)
// For ESP32-C3 and ESP32-C6, specifying the clock and data pins helps resolve I2C initialisation issues
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);
#else
// use default I2C
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
#endif

TCA9548 multiplexer(0x70); // Default Pa.HUB address

DHT dht(DHTPIN, DHTTYPE);
SensirionI2CSgp41 sgp41;
HM330X hm330x;
SPL07_003 spa06;

WebServer server(80);

// --- Settings Management ---
Preferences preferences;
WiFiMulti wifiMulti;
bool isSetupMode = false;

// --- MQTT Configuration ---
String mqtt_server = "";
int mqtt_port = 1883;
String mqtt_user = "";
String mqtt_password = "";

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

void setupSensors();
void readSensors();
void loadSettings();
void startSetupPortal();
void handleSetupRoot();
void handleSetupSave();

void updateOLED();
void reconnectMQTT();
void setupWebServer();
void handleRoot();
void handleApiData();
void handleSaveConfig();
int checkAuth();

void loadSettings() {
  preferences.begin("airquality", false);
  
  // Load WiFi Networks
  String ssid1 = preferences.getString("ssid1", "");
  String pass1 = preferences.getString("pass1", "");
  String ssid2 = preferences.getString("ssid2", "");
  String pass2 = preferences.getString("pass2", "");
  
  if(ssid1.length() > 0) wifiMulti.addAP(ssid1.c_str(), pass1.c_str());
  if(ssid2.length() > 0) wifiMulti.addAP(ssid2.c_str(), pass2.c_str());

  // Load MQTT Settings
  mqtt_server = preferences.getString("mqtt_ip", "");
  mqtt_port = preferences.getInt("mqtt_port", 1883);
  mqtt_user = preferences.getString("mqtt_user", "");
  mqtt_password = preferences.getString("mqtt_pass", "");
  
  preferences.end();
}

void handleSetupRoot() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'><style>body{font-family:sans-serif;padding:20px;background:#1a1a2e;color:#fff;} input,button{padding:10px;margin-top:5px;width:100%;box-sizing:border-box;border-radius:5px;} button{background:#e94560;color:#fff;border:none;cursor:pointer;} .card{background:#16213e;padding:20px;border-radius:10px;}</style></head><body>";
  html += "<h2>AirQuality Setup</h2><div class='card'>";
  html += "<form action='/save' method='POST'>";
  html += "<h3>WiFi Network 1</h3>";
  html += "<input type='text' name='ssid1' placeholder='SSID 1'><br>";
  html += "<input type='password' name='pass1' placeholder='Password 1'><br>";
  html += "<h3>WiFi Network 2 (Optional Fallback)</h3>";
  html += "<input type='text' name='ssid2' placeholder='SSID 2'><br>";
  html += "<input type='password' name='pass2' placeholder='Password 2'><br>";
  html += "<h3>WebUI Credentials</h3>";
  html += "<input type='text' name='admin_user' placeholder='Admin Username' value='admin'><br>";
  html += "<input type='password' name='admin_pass' placeholder='Admin Password (password123)'><br>";
  html += "<input type='text' name='read_user' placeholder='Read-Only Username' value='user'><br>";
  html += "<input type='password' name='read_pass' placeholder='Read-Only Password (read123)'><br>";
  html += "<h3>MQTT Broker</h3>";
  html += "<input type='text' name='mqtt_ip' placeholder='Broker IP (e.g. 192.168.1.100)'><br>";
  html += "<input type='number' name='mqtt_port' placeholder='Port (1883)' value='1883'><br>";
  html += "<input type='text' name='mqtt_user' placeholder='MQTT Username'><br>";
  html += "<input type='password' name='mqtt_pass' placeholder='MQTT Password'><br>";
  html += "<button type='submit'>Save and Reboot</button>";
  html += "</form></div></body></html>";
  server.send(200, "text/html", html);
}

void handleSetupSave() {
  preferences.begin("airquality", false);
  if(server.hasArg("ssid1")) preferences.putString("ssid1", server.arg("ssid1"));
  if(server.hasArg("pass1")) preferences.putString("pass1", server.arg("pass1"));
  if(server.hasArg("ssid2")) preferences.putString("ssid2", server.arg("ssid2"));
  if(server.hasArg("pass2")) preferences.putString("pass2", server.arg("pass2"));
  
  if(server.hasArg("admin_user")) preferences.putString("admin_user", server.arg("admin_user"));
  if(server.hasArg("admin_pass")) preferences.putString("admin_pass", server.arg("admin_pass"));
  if(server.hasArg("read_user")) preferences.putString("read_user", server.arg("read_user"));
  if(server.hasArg("read_pass")) preferences.putString("read_pass", server.arg("read_pass"));

  if(server.hasArg("mqtt_ip")) preferences.putString("mqtt_ip", server.arg("mqtt_ip"));
  if(server.hasArg("mqtt_port")) preferences.putInt("mqtt_port", server.arg("mqtt_port").toInt());
  if(server.hasArg("mqtt_user")) preferences.putString("mqtt_user", server.arg("mqtt_user"));
  if(server.hasArg("mqtt_pass")) preferences.putString("mqtt_pass", server.arg("mqtt_pass"));
  preferences.end();
  
  server.send(200, "text/html", "<html><body><h2>Saved! Rebooting...</h2></body></html>");
  delay(2000);
  ESP.restart();
}

void handleSettingsSave() {
  // Only Admin can save settings
  if (checkAuth() != 1) {
    server.send(403, "text/plain", "Forbidden - Admin access required.");
    return;
  }
  
  handleSetupSave(); // Use same logic to save to NVS and reboot
}

void startSetupPortal() {
  isSetupMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP("AirQuality_Setup", "password123");
  Serial.println("AP Started: AirQuality_Setup / password123");
  Serial.print("Web Portal IP: ");
  Serial.println(WiFi.softAPIP());
  
  u8x8.clear();
  u8x8.setCursor(0,0);
  u8x8.println("Setup Connect:");
  u8x8.println("AirQuality_Setup");
  u8x8.println("password123");
  u8x8.println(WiFi.softAPIP().toString());
  
  server.on("/", HTTP_GET, handleSetupRoot);
  server.on("/save", HTTP_POST, handleSetupSave);
  server.begin();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nStarting Air Quality Monitor ESP32-C6...");

  // 1. Initialize Display First (to show connection status)
  Wire.begin();
  u8x8.begin();
  u8x8.setFlipMode(1);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();
  u8x8.setCursor(0, 0);
  u8x8.println("Booting...");
  u8x8.println("Starting WiFiSetup");

  // 2. Load Settings & Initialize WiFi
  loadSettings();
  
  WiFi.mode(WIFI_STA);
  Serial.println("Connecting to WiFi using WiFiMulti...");
  
  // Try to connect up to 15 seconds
  int attempts = 0;
  while (wifiMulti.run() != WL_CONNECTED && attempts < 15) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi! Starting Setup AP.");
    startSetupPortal();
    return; // Don't proceed to sensors/MQTT in setup mode
  }

  // 3. Initialize Sensors
  setupSensors();

  if (mqtt_server.length() > 0) {
    mqttClient.setServer(mqtt_server.c_str(), mqtt_port);
  } // 5. Initialize Web Server
  setupWebServer();
  server.begin();
  Serial.println("HTTP Server Started");
}

void reconnectMQTT() {
  if (mqttClient.connected() || mqtt_server.length() == 0)
    return;
  Serial.print("Attempting MQTT connection to ");
  Serial.print(mqtt_server);
  Serial.print("...");
  
  mqttClient.setServer(mqtt_server.c_str(), mqtt_port);
  
  if (mqttClient.connect("ESP32_C6_AirQuality", mqtt_user.c_str(), mqtt_password.c_str())) {
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
      JsonDocument doc;
      doc["name"] = String("AirQuality ") + sensors[i].name;
      doc["state_topic"] = state_topic;
      if (strlen(sensors[i].unit) > 0)
        doc["unit_of_measurement"] = sensors[i].unit;
      doc["device_class"] = sensors[i].device_class;
      doc["value_template"] = sensors[i].val_tpl;
      doc["unique_id"] = String("airquality_c6_") + sensors[i].id;

      JsonObject device = doc["device"].to<JsonObject>();
      JsonArray identifiers = device["identifiers"].to<JsonArray>();
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
  server.handleClient();
  if (isSetupMode) {
    return; // Stay in AP mode and do nothing else
  }

  if (wifiMulti.run() != WL_CONNECTED) {
    // Attempting auto reconnect via wifiMulti
  }

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
      JsonDocument doc;
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
  u8x8.clear();
  u8x8.setCursor(0, 0);

  u8x8.println(WiFi.localIP().toString());
  
  u8x8.print("T:");u8x8.print(tempC, 1);u8x8.print("C H:");u8x8.print(humidity, 0);u8x8.println("%");
  u8x8.print("PM2.5:");u8x8.print(pm25);u8x8.println(" ug/m3");
  u8x8.print("VOC:");u8x8.println(srawVoc);
  u8x8.print("Prs:");u8x8.print(pressure_hPa, 1);u8x8.println("hPa");
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/data", HTTP_GET, handleApiData);
  server.on("/settings/save", HTTP_POST, handleSettingsSave);
}

int checkAuth() {
  preferences.begin("airquality", true);
  String admin_user = preferences.getString("admin_user", "admin");
  String admin_pass = preferences.getString("admin_pass", "password123");
  String read_user = preferences.getString("read_user", "user");
  String read_pass = preferences.getString("read_pass", "read123");
  preferences.end();

  if (server.authenticate(admin_user.c_str(), admin_pass.c_str())) {
    return 1; // Admin
  }
  if (server.authenticate(read_user.c_str(), read_pass.c_str())) {
    return 2; // Read-Only
  }

  server.requestAuthentication();
  return 0; // Unauthorized
}

void handleRoot() {
  int role = checkAuth();
  if (role == 0)
    return; // 0 is unauthorized

  bool isAdmin = (role == 1);

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
    --green: #4ade80;
    --yellow: #facc15;
    --red: #f87171;
  }
  body {
    background: var(--bg); color: #fff; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    margin: 0; padding: 20px;
  }
  .container { max-width: 800px; margin: auto; }
  .tabs { display: flex; gap: 10px; margin-bottom: 20px; flex-wrap: wrap; }
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
  .card h3 { margin: 0 0 5px 0; color: #a2a2bd; font-size: 14px; }
  .card .val { font-size: 24px; font-weight: bold; color: var(--text); }
  .card .range { font-size: 11px; color: #888; margin-top: 5px; }
  .good { color: var(--green) !important; text-shadow: 0 0 5px rgba(74,222,128,0.5); }
  .warn { color: var(--yellow) !important; text-shadow: 0 0 5px rgba(250,204,21,0.5); }
  .danger { color: var(--red) !important; text-shadow: 0 0 5px rgba(248,113,113,0.5); }
  input, button { padding: 10px; margin-top: 10px; border-radius: 5px; border: none; }
  button { background: var(--text); color: white; cursor: pointer; transition: 0.2s; }
  button:hover { opacity: 0.8; }
  .info-table { width: 100%; border-collapse: collapse; margin-top: 10px; }
  .info-table th, .info-table td { padding: 10px; border-bottom: 1px solid #333; text-align: left; }
</style>
</head>
<body>
<div class="container">
  <h2>Air Quality Monitor <span style="font-size:12px;color:#aaa;">()rawliteral";
  
  html += isAdmin ? "Admin" : "Read-Only Viewer";
  html += R"rawliteral()</span></h2>
  
  <div class="tabs">
    <div class="tab active" onclick="showTab('dash')">Dashboard</div>
    <div class="tab" onclick="showTab('info')">Detailed Info</div>
)rawliteral";

  if (isAdmin) {
    html += R"rawliteral(
    <div class="tab" onclick="showTab('settings')">Settings</div>
    <div class="tab" onclick="showTab('calib')">Calibration</div>
    )rawliteral";
  }

  html += R"rawliteral(
  </div>

  <div id="dash" class="panel active">
    <div class="grid">
      <div class="card">
        <h3>Temperature</h3>
        <div class="val" id="t-dht">-- &deg;C</div>
        <div class="range">Ideal: 18-24 &deg;C</div>
      </div>
      <div class="card">
        <h3>Humidity</h3>
        <div class="val" id="h-dht">-- %</div>
        <div class="range">Ideal: 30-60 %</div>
      </div>
      <div class="card">
        <h3>PM 2.5</h3>
        <div class="val" id="v-pm">-- &micro;g/m&sup3;</div>
        <div class="range">Good: &lt; 12 | Warn: 12-35</div>
      </div>
      <div class="card">
        <h3>VOC (Raw)</h3>
        <div class="val" id="v-voc">--</div>
        <div class="range">Baseline ~ 100-300</div>
      </div>
      <div class="card">
        <h3>NOx (Raw)</h3>
        <div class="val" id="v-nox">--</div>
        <div class="range">Lower is better</div>
      </div>
    </div>
  </div>

  <div id="info" class="panel">
    <h3>Data Ranges & Details</h3>
    <table class="info-table">
      <tr><th>Sensor</th><th>Good (Green)</th><th>Caution (Yellow)</th><th>Bad (Red)</th></tr>
      <tr><td>Temperature</td><td>18 - 26 °C</td><td>10-18 °C or 26-30 °C</td><td>< 10 or > 30 °C</td></tr>
      <tr><td>Humidity</td><td>30 - 60 %</td><td>20-30 % or 60-70 %</td><td>< 20 or > 70 %</td></tr>
      <tr><td>PM 2.5</td><td>0 - 12 µg/m³</td><td>12 - 35 µg/m³</td><td>> 35 µg/m³</td></tr>
    </table>
    <p style="font-size:12px; color:#aaa; margin-top:20px;">* VOC and NOx are raw ticks from the SGP41. They require a baseline calibration. Higher numbers indicates heavier concentration.</p>
  </div>
)rawliteral";

  if (isAdmin) {
    html += R"rawliteral(
  <div id="settings" class="panel">
    <h3>System Settings</h3>
    <p>Admin Configuration controls. Warning: saving will reboot the device.</p>
    <form action='/settings/save' method='POST'>
      <div style="margin-bottom:15px">
        <h4>WiFi Network 1</h4>
        <input type='text' name='ssid1' placeholder='SSID 1'><br>
        <input type='password' name='pass1' placeholder='Password 1'>
      </div>
      <div style="margin-bottom:15px">
        <h4>WiFi Network 2 (Optional Fallback)</h4>
        <input type='text' name='ssid2' placeholder='SSID 2'><br>
        <input type='password' name='pass2' placeholder='Password 2'>
      </div>
      <div style="margin-bottom:15px">
        <h4>WebUI Credentials</h4>
        <input type='text' name='admin_user' placeholder='Admin Username (default: admin)'><br>
        <input type='password' name='admin_pass' placeholder='Admin Password (default: password123)'><br>
        <input type='text' name='read_user' placeholder='Read-Only Username (default: user)'><br>
        <input type='password' name='read_pass' placeholder='Read-Only Password (default: read123)'>
      </div>
      <div style="margin-bottom:15px">
        <h4>MQTT Broker</h4>
        <input type='text' name='mqtt_ip' placeholder='Broker IP (e.g. 192.168.1.100)'><br>
        <input type='number' name='mqtt_port' placeholder='Port (1883)' value='1883'><br>
        <input type='text' name='mqtt_user' placeholder='MQTT Username'><br>
        <input type='password' name='mqtt_pass' placeholder='MQTT Password'>
      </div>
      <button type="submit">Save and Reboot</button>
    </form>
  </div>

  <div id="calib" class="panel">
    <h3>Sensor Calibration</h3>
    <p>Trigger internal sensor baselines and tests.</p>
    <button onclick="runCalib('sgp41')">Reset SGP41 Baseline</button>
    <button onclick="runCalib('hm3301')">Trigger HM3301 Cleaning</button>
  </div>
    )rawliteral";
  }

  html += R"rawliteral(
</div>

<script>
  function showTab(id) {
    document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
    document.getElementById(id).classList.add('active');
    event.target.classList.add('active');
  }

  function applyColor(elementId, value, goodMax, warnMax, inverted = false) {
    const el = document.getElementById(elementId);
    el.className = 'val'; // reset
    if (!inverted) {
      if (value <= goodMax) el.classList.add('good');
      else if (value <= warnMax) el.classList.add('warn');
      else el.classList.add('danger');
    } else {
      // For things like humidity where too low is also bad (ideal 30-60)
      if (value >= 30 && value <= 60) el.classList.add('good');
      else if (value >= 20 && value <= 70) el.classList.add('warn');
      else el.classList.add('danger');
    }
  }

  function applyColorTemp(elementId, temp) {
     const el = document.getElementById(elementId);
     el.className = 'val';
     if (temp >= 18 && temp <= 26) el.classList.add('good');
     else if ((temp >= 10 && temp < 18) || (temp > 26 && temp <= 30)) el.classList.add('warn');
     else el.classList.add('danger');
  }

  async function updateData() {
    try {
      const res = await fetch('/api/data');
      if (res.status === 401) return; 
      const data = await res.json();
      
      document.getElementById('t-dht').innerText = data.temperature.toFixed(1) + ' °C';
      applyColorTemp('t-dht', data.temperature);

      document.getElementById('h-dht').innerText = data.humidity.toFixed(1) + ' %';
      applyColor('h-dht', data.humidity, 0, 0, true);

      document.getElementById('v-pm').innerText = data.pm2_5 + ' µg/m³';
      applyColor('v-pm', data.pm2_5, 12, 35);

      document.getElementById('v-voc').innerText = data.voc_raw;
      applyColor('v-voc', data.voc_raw, 250, 400); // Rough raw estimates

      document.getElementById('v-nox').innerText = data.nox_raw;
      applyColor('v-nox', data.nox_raw, 250, 400); 

    } catch(e) { console.error('Fetch error:', e); }
  }
  
  async function runCalib(sensor) {
    alert("Sent calibration command for: " + sensor + " (Mock Implementation)");
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
  if (checkAuth() == 0)
    return; // Secure API endpoint as well

  JsonDocument doc;
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
