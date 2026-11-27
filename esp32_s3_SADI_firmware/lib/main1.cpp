#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <DHT.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ============================================================
// HARDWARE
// ============================================================
#define Ventilador                      5
#define Sensor_DTH22                    44
#define Sensor_DTH22_Ambiental          43
#define Calentador_Temperatura          15
#define Sistema_Dosificador_de_Humedad  16

#define TIPO_DHT DHT22
#define PERIODO_DHT_MS   2000
#define PERIODO_PID_MS   500
#define PERIODO_WS_MS    100
#define WS_PORT          81
#define DNS_PORT         53

const char* AP_SSID = "Wifi_Config_SADI";
const char* AP_PASS = "";

// ============================================================
// OBJETOS GLOBALES
// ============================================================
DHT dht1(Sensor_DTH22, TIPO_DHT);
DHT dht2(Sensor_DTH22_Ambiental, TIPO_DHT);
WebServer webServer(80);
DNSServer dnsServer;
Preferences preferences;
WebSocketsServer wsServer(WS_PORT);

// ============================================================
// ESTADO COMPARTIDO
// ============================================================
volatile float pvTemp = NAN;
volatile float pvHum  = NAN;
volatile float pvAmb  = NAN;
portMUX_TYPE muxSensors = portMUX_INITIALIZER_UNLOCKED;

float sp = 37.5;
float kp = 0.155;
float ti = 150.0;
float td = 0.0;
bool  autoMode = true;
bool  actVentilador  = false;
bool  actCalentador  = false;
bool  actDosificador = false;
float controlOut = 0;

float X0_PID_Temperatura[4] = {0, 0, 0, 0};
float Y0_PID_Temperatura[4] = {0, 0, 0, 0};

String ssidGuardado, passGuardado;
unsigned long ultimoPID = 0, ultimoWS = 0;

// ============================================================
// MODO SIMULACIÓN
// ============================================================
bool  modoSimulacion = false;
float simTemp = 25.0f;   // °C inicial
float simHum  = 55.0f;   // % inicial
const float SIM_AMBIENTE_TEMP = 22.0f;
const float SIM_AMBIENTE_HUM  = 45.0f;

// ============================================================
// PID
// ============================================================
void calcularPIDTemp(float entrada) {
  const float T   = PERIODO_PID_MS / 1000.0;
  const float Kid = (kp * T) / ti;
  const float Kdd = (kp * td) / T;
  const float Kpd = kp;

  X0_PID_Temperatura[0] = sp - entrada;
  Y0_PID_Temperatura[0] = X0_PID_Temperatura[0] * (Kpd + Kid + Kdd)
                        - X0_PID_Temperatura[1] * (Kpd + 2 * Kdd)
                        + X0_PID_Temperatura[2] * Kdd
                        + Y0_PID_Temperatura[1];

  if (Y0_PID_Temperatura[0] >  100) Y0_PID_Temperatura[0] =  100;
  if (Y0_PID_Temperatura[0] < -100) Y0_PID_Temperatura[0] = -100;

  for (int i = 3; i > 0; i--) {
    X0_PID_Temperatura[i] = X0_PID_Temperatura[i - 1];
    Y0_PID_Temperatura[i] = Y0_PID_Temperatura[i - 1];
  }
  controlOut = Y0_PID_Temperatura[0];
}

// ============================================================
// PLANTA SIMULADA (modelo de 1er orden + ruido)
// ============================================================
void actualizarSimulacion(float dtSeg) {
  float heater = actCalentador ? 1.0f : 0.0f;
  float cooler = actVentilador ? 1.0f : 0.0f;
  float humid  = actDosificador ? 1.0f : 0.0f;

  // Calentamiento activo vs enfriamiento y pérdida pasiva hacia ambiente
  simTemp += (heater * 3.0f - cooler * 2.5f
              - (simTemp - SIM_AMBIENTE_TEMP) * 0.05f) * dtSeg;
  simHum  += (humid * 4.0f
              - (simHum - SIM_AMBIENTE_HUM) * 0.03f) * dtSeg;

  // Ruido gaussiano aproximado (±0.25 °C, ±2.5 %)
  simTemp += ((random(0, 100) - 50) / 200.0f);
  simHum  += ((random(0, 100) - 50) / 20.0f);

  // Clamps físicos
  if (simTemp < 15.0f) simTemp = 15.0f;
  if (simTemp > 60.0f) simTemp = 60.0f;
  if (simHum  < 20.0f) simHum  = 20.0f;
  if (simHum  > 95.0f) simHum  = 95.0f;
}

// ============================================================
// TAREA DHT + SIMULACIÓN (CORE 0)
// ============================================================
void tareaSensores(void*) {
  TickType_t ultimo = xTaskGetTickCount();
  for (;;) {
    float t1, h1, t2;

    if (modoSimulacion) {
      float dt = PERIODO_DHT_MS / 1000.0f;
      actualizarSimulacion(dt);
      t1 = simTemp;
      h1 = simHum;
      t2 = SIM_AMBIENTE_TEMP + ((random(0, 100) - 50) / 500.0f);
    } else {
      t1 = dht1.read;
      h1 = dht1.readHumidity();
      t2 = dht2.readTemperature();
    }

    portENTER_CRITICAL(&muxSensors);
    if (!isnan(t1)) pvTemp = t1;
    if (!isnan(h1)) pvHum  = h1;
    if (!isnan(t2)) pvAmb  = t2;
    portEXIT_CRITICAL(&muxSensors);

    vTaskDelayUntil(&ultimo, pdMS_TO_TICKS(PERIODO_DHT_MS));
  }
}

// ============================================================
// WEBSOCKET — helpers
// ============================================================
void enviarEstado(uint8_t client) {
  StaticJsonDocument<512> doc;
  doc["type"] = "state";
  JsonObject p = doc.createNestedObject("payload");

  float temp, hum;
  portENTER_CRITICAL(&muxSensors);
  temp = pvTemp; hum = pvHum;
  portEXIT_CRITICAL(&muxSensors);

  p["sp"]          = sp;
  p["kp"]          = kp;
  p["ti"]          = ti;
  p["td"]          = td;
  p["mode"]        = autoMode ? "auto" : "manual";
  p["sim"]         = modoSimulacion;
  p["temp"]        = isnan(temp) ? 0.0 : temp;
  p["hum"]         = isnan(hum)  ? 0.0 : hum;
  p["output"]      = controlOut;
  p["ventilador"]  = actVentilador;
  p["calentador"]  = actCalentador;
  p["dosificador"] = actDosificador;

  String out; serializeJson(doc, out);
  wsServer.sendTXT(client, out);
}

void enviarAck(uint8_t c, bool ok, const char* cmd, const char* msg = nullptr) {
  StaticJsonDocument<256> d;
  d["type"] = "ack";
  d["ok"]   = ok;
  d["cmd"]  = cmd;
  if (msg) d["msg"] = msg;
  String o; serializeJson(d, o);
  wsServer.sendTXT(c, o);
}

void enviarError(uint8_t c, const char* msg) {
  StaticJsonDocument<128> d;
  d["type"] = "error";
  d["msg"]  = msg;
  String o; serializeJson(d, o);
  wsServer.sendTXT(c, o);
}

// ============================================================
// WEBSOCKET — comandos
// ============================================================
void manejarComando(uint8_t client, const String& txt) {
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, txt)) { enviarError(client, "JSON inválido"); return; }
  const char* type = doc["type"] | "";

  if (strcmp(type, "get_state") == 0) {
    enviarEstado(client);
  }
  else if (strcmp(type, "set_sim") == 0) {
    bool nuevo = doc["value"] | false;
    if (nuevo != modoSimulacion) {
      modoSimulacion = nuevo;
      if (nuevo) {
        simTemp = 25.0f;
        simHum  = 55.0f;
      }
      Serial.printf("Modo simulación: %s\n", nuevo ? "ON" : "OFF");
    }
    enviarAck(client, true, "set_sim");
    enviarEstado(client);
  }
  else if (strcmp(type, "set_pid") == 0) {
    if (doc.containsKey("sp")) { float v = doc["sp"]; if (v >= 0 && v <= 100)  sp = v; }
    if (doc.containsKey("kp")) { float v = doc["kp"]; if (v >= 0 && v <= 5)    kp = v; }
    if (doc.containsKey("ti")) { float v = doc["ti"]; if (v >= 1 && v <= 1000) ti = v; }
    if (doc.containsKey("td")) { float v = doc["td"]; if (v >= 0 && v <= 100)  td = v; }
    enviarAck(client, true, "set_pid");
    enviarEstado(client);
  }
  else if (strcmp(type, "set_mode") == 0) {
    const char* m = doc["mode"] | "";
    if      (strcmp(m, "auto")   == 0) autoMode = true;
    else if (strcmp(m, "manual") == 0) autoMode = false;
    else { enviarAck(client, false, "set_mode", "modo inválido"); return; }
    enviarAck(client, true, "set_mode");
    enviarEstado(client);
  }
  else if (strcmp(type, "set_actuator") == 0) {
    if (autoMode) { enviarAck(client, false, "set_actuator", "auto activo"); return; }
    const char* name = doc["name"] | "";
    bool value = doc["value"] | false;
    if      (strcmp(name, "ventilador")  == 0) actVentilador  = value;
    else if (strcmp(name, "calentador")  == 0) actCalentador  = value;
    else if (strcmp(name, "dosificador") == 0) actDosificador = value;
    else { enviarAck(client, false, "set_actuator", "desconocido"); return; }
    digitalWrite(Ventilador,                     actVentilador  ? HIGH : LOW);
    digitalWrite(Calentador_Temperatura,         actCalentador  ? HIGH : LOW);
    digitalWrite(Sistema_Dosificador_de_Humedad, actDosificador ? HIGH : LOW);
    enviarAck(client, true, "set_actuator");
    enviarEstado(client);
  }
  else {
    enviarError(client, "comando desconocido");
  }
}

void onWsEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.printf("WS cliente %u conectado\n", num);
    enviarEstado(num);
  }
  else if (type == WStype_TEXT) {
    manejarComando(num, String((char*)payload));
  }
}

// ============================================================
// WIFI + PORTAL
// ============================================================
bool conectarWiFi(const String& ssid, const String& pass) {
  WiFi.begin(ssid.c_str(), pass.c_str());
  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 30) delay(500);
  return WiFi.status() == WL_CONNECTED;
}

String contentTypeFor(const String& p) {
  if (p.endsWith(".html")) return "text/html";
  if (p.endsWith(".css"))  return "text/css";
  if (p.endsWith(".js"))   return "application/javascript";
  if (p.endsWith(".json")) return "application/json";
  if (p.endsWith(".png"))  return "image/png";
  if (p.endsWith(".jpg") || p.endsWith(".jpeg")) return "image/jpeg";
  if (p.endsWith(".webp")) return "image/webp";
  if (p.endsWith(".svg"))  return "image/svg+xml";
  if (p.endsWith(".ico"))  return "image/x-icon";
  return "application/octet-stream";
}

bool servirDeLittleFS(String path) {
  if (path.endsWith("/")) path += "index.html";
  String real = path;
  if (!LittleFS.exists(real) && LittleFS.exists("/data" + path))
    real = "/data" + path;
  if (!LittleFS.exists(real)) return false;
  File f = LittleFS.open(real, "r");
  if (!f) return false;
  webServer.streamFile(f, contentTypeFor(real));
  f.close();
  return true;
}

// ============================================================
// RUTAS HTTP
// ============================================================
void manejarRaiz() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!servirDeLittleFS("/scope.html")) {
      webServer.send(500, "text/plain",
        "Falta /scope.html en LittleFS. Ejecuta 'pio run -t uploadfs'.");
    }
    return;
  }
  if (!servirDeLittleFS("/WIFI/Interfaz_WiFi.html")) {
    webServer.send(500, "text/plain",
      "Falta /WIFI/Interfaz_WiFi.html en LittleFS. Ejecuta 'pio run -t uploadfs'.");
  }
}

void manejarGuardar() {
  if (!webServer.hasArg("ssid") || !webServer.hasArg("password")) {
    webServer.send(400, "text/plain", "Faltan parámetros");
    return;
  }
  String s = webServer.arg("ssid");
  String p = webServer.arg("password");

  preferences.begin("wifi", false);
  preferences.putString("ssid", s);
  preferences.putString("password", p);
  preferences.end();

  if (conectarWiFi(s, p))
    webServer.send(200, "text/plain", "✅ Conectado. IP: " + WiFi.localIP().toString());
  else
    webServer.send(200, "text/plain", "❌ Error de conexión.");
}

void manejarEscanear() {
  String json = "[";
  int n = WiFi.scanComplete();
  if (n == -2) {
    WiFi.scanNetworks(true);
    json = "[]";
  } else if (n >= 0) {
    for (int i = 0; i < n; ++i) {
      if (i) json += ",";
      json += "\"" + WiFi.SSID(i) + "\"";
    }
    WiFi.scanDelete();
  }
  json += "]";
  webServer.send(200, "application/json", json);
}

void manejarArchivo() {
  if (!servirDeLittleFS(webServer.uri()))
    webServer.send(404, "text/plain", "No encontrado: " + webServer.uri());
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  randomSeed(esp_random());

  dht1.begin();
  dht2.begin();

  pinMode(Ventilador, OUTPUT);
  pinMode(Calentador_Temperatura, OUTPUT);
  pinMode(Sistema_Dosificador_de_Humedad, OUTPUT);
  pinMode(Sensor_DTH22, INPUT);
  pinMode(Sensor_DTH22_Ambiental, INPUT);

  digitalWrite(Ventilador, LOW);
  digitalWrite(Calentador_Temperatura, LOW);
  digitalWrite(Sistema_Dosificador_de_Humedad, LOW);

  if (!LittleFS.begin(true)) Serial.println("LittleFS mount falló");

  preferences.begin("wifi", false);
  ssidGuardado = preferences.getString("ssid", "");
  passGuardado = preferences.getString("password", "");
  preferences.end();

  webServer.on("/",         manejarRaiz);
  webServer.on("/guardar",  HTTP_POST, manejarGuardar);
  webServer.on("/escanear", HTTP_GET,  manejarEscanear);
  webServer.onNotFound(manejarArchivo);
  webServer.begin();

  bool ok = false;
  if (ssidGuardado != "") ok = conectarWiFi(ssidGuardado, passGuardado);

  if (!ok) {
    Serial.println("Portal AP: " + String(AP_SSID));
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASS);
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    while (WiFi.status() != WL_CONNECTED) {
      dnsServer.processNextRequest();
      webServer.handleClient();
      delay(10);
    }
    dnsServer.stop();
  }

  Serial.print("WiFi OK · IP: "); Serial.println(WiFi.localIP());
  Serial.print("Abre: http://");  Serial.print(WiFi.localIP()); Serial.println("/");
  Serial.println("Modo simulación disponible vía WS: {type:'set_sim', value:true|false}");

  wsServer.begin();
  wsServer.onEvent(onWsEvent);

  xTaskCreatePinnedToCore(tareaSensores, "DHT", 4096, nullptr, 1, nullptr, 0);

  ultimoPID = millis();
  ultimoWS  = millis();
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  webServer.handleClient();
  wsServer.loop();

  unsigned long ahora = millis();

  if (ahora - ultimoPID >= PERIODO_PID_MS) {
    ultimoPID = ahora;

    float temp;
    portENTER_CRITICAL(&muxSensors);
    temp = pvTemp;
    portEXIT_CRITICAL(&muxSensors);

    if (!isnan(temp)) {
      calcularPIDTemp(temp);
      if (autoMode) {
        float err = sp - temp;
        if      (err >  1.0) { actCalentador = true;  actVentilador = false; }
        else if (err < -1.0) { actCalentador = false; actVentilador = true;  }
        else                 { actCalentador = false; actVentilador = false; }
        digitalWrite(Ventilador,             actVentilador ? HIGH : LOW);
        digitalWrite(Calentador_Temperatura, actCalentador ? HIGH : LOW);
      }
    }
  }

  if (ahora - ultimoWS >= PERIODO_WS_MS) {
    ultimoWS = ahora;
    if (wsServer.connectedClients() > 0) {
      float temp;
      portENTER_CRITICAL(&muxSensors);
      temp = pvTemp;
      portEXIT_CRITICAL(&muxSensors);

      float t   = isnan(temp) ? 0.0f : temp;
      float err = sp - t;

      int16_t frame[4];
      frame[0] = (int16_t)(sp         * 100);
      frame[1] = (int16_t)(t          * 100);
      frame[2] = (int16_t)(err        * 100);
      frame[3] = (int16_t)(controlOut *  10);

      wsServer.broadcastBIN((uint8_t*)frame, 8);
    }
  }
}