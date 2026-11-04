#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <math.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <semphr.h>
#include <DHT.h>

// Configuración de pines.
#define TIPO_DHT DHT22
#define Ventilador 5
#define Sensor_DHT22 44
#define Sensor_DHT22_Ambiental 43
#define Calentador_Temperatura 15
#define Sistema_Dosificador_de_Humedad 16
#define PIN_LPWM = 16
#define PIN_ENC_B = 26

bool WiFi_Conectado = false;

const byte DNS_PORT = 53;

// Creacion de identificadores:

DHT dht1(Sensor_DHT22, TIPO_DHT);
DHT dht2(Sensor_DHT22_Ambiental, TIPO_DHT);

DNSServer dnsServer;
WebServer webServer(80);
Preferences preferences;

String usuarios_Permitidos[20];

String Nombre_Wifi_Guardado = "";
String Password_Wifi_Guardado = "";

// Configuración Puente H
constexpr int FREQ = 15000, RESOLUTION = 8;
constexpr int CH_RPWM = 0, CH_LPWM = 1;

float Feed_Forward_Temperatura, Feed_Forward_Humedad, Setpoint, Temperatura_Ambiental;

float aoT1 = 1 - exp(-Tiempo_de_Muestreo / Tiempo_de_Subida_Sistema), a1T1 = 0, a2T1 = 0;
float b0T1 = 0, b1T1 = 0, b2T1 = 0, b3T1 = 0;

float b0H1 = 0, b1H1 = 0, b2H1 = 0, b3H1 = 0;
float aoH1 = 0, a1H1 = 0, a2H1 = 0;

// Configuración de ecuaciones de diferencias (Temperatura).
float a0_T1[] = {aoT1, a1T1, a2T1};
float b0_T1[] = {b0T1, b1T1, b2T1, b3T1};

float aoT1 = 0, a1T1 = 0, a2T1 = 0;
float b0T1 = 0, b1T1 = 0, b2T1 = 0, b3T1 = 0;
// Configuración de ecuaciones de diferencias (Humedad).

float a0_H1[] = {aoH1, a1H1, a2H1};
float b0_H1[] = {b0H1, b1H1, b2H1, b3H1};

float X0_Temperatura[4] = {0, 0, 0, 0};
float Y0_Temperatura[4] = {0, 0, 0, 0};

float X0_Humedad[4] = {0, 0, 0, 0};
float Y0_Humedad[4] = {0, 0, 0, 0};

float X0_PID_Temperatura[4] = {0, 0, 0, 0};
float Y0_PID_Temperatura[4] = {0, 0, 0, 0};
float X0_PID_Humedad[4] = {0, 0, 0, 0};
float Y0_PID_Humedad[4] = {0, 0, 0, 0};

// Configuración de los tiempos del sistema.
constexpr double Tiempo_Maximo_Ejecucion = 1.2e3, Tiempo_de_Muestreo = 2;

constexpr double Tiempo_de_Subida_Sistema_Temperatura = 150.0,
                 Tiempo_de_Retardo_Sistema_Temperatura = 4.2,
                 Tiempo_de_Subida_Sensor_Temperatura = 26.38,
                 Tiempo_de_Retardo_Sensor_Temperatura = 5.0;

constexpr double Tiempo_de_Subida_Sistema_Humedad = 100.0,
                 Tiempo_de_Retardo_Sistema_Humedad = 4.2,
                 Tiempo_de_Subida_Sensor_Humedad = 26.38,
                 Tiempo_de_Retardo_Sensor_Humedad = 5.0;

constexpr double K_Temperatura = 30.0,
                 K_Humedad = 40, 
                 Setpoint_Temperatura = 37.5,
                 Setpoint_Humedad=60;

volatile float Point_Value_Humedad_Incubadora = NAN, Point_Value_Temperatura_Incubadora = NAN, Point_Value_Temperatura_Ambiental = NAN;

portMUX_TYPE muxSensors = portMUX_INITIALIZER_UNLOCKED;

// Configuración del controlador PID.
// El metodo de sintonización es el metodo IMC
void Configuracion_PID(float Tau, float Retardo_sys, float Retardo_Sensor, float Gananica_Planta)
{
  constexpr double Kp = (Tau * 0.6) /
                        (Gananica_Planta * (Retardo_Sensor + Retardo_sys + Tiempo_de_Muestreo) * 2.0),
                   Ti = Tiempo_de_Subida_Sistema,
                   Td = 0.0,

                   // El sistema determina los valores PID Discretos.
      Kid = (Kp * Tiempo_de_Muestreo) / Ti,
                   Kdd = (Kp * Td) / Tiempo_de_Muestreo,
                   Kpd = Kp;
  return Kid, Kdd, Kpd;
}

constexpr int Nd = 1;

unsigned long Tiempo_Actual, Tiempo_Anterior, chars;

void Controlar_Foco(int pwmIzq, int pwmDer, int dirIzq, int dirDer, int velocidad, bool girarIzq, bool girarDer)
{
  if (girarIzq && !girarDer)
  {
    digitalWrite(dirIzq, HIGH);
    digitalWrite(dirDer, LOW);
    ledcWrite(pwmIzq, velocidad);
    ledcWrite(pwmDer, 0);
  }
  else if (!girarIzq && girarDer)
  {
    digitalWrite(dirIzq, LOW);
    digitalWrite(dirDer, HIGH);
    ledcWrite(pwmIzq, 0);
    ledcWrite(pwmDer, velocidad);
  }
  else
  {
    digitalWrite(dirIzq, LOW);
    digitalWrite(dirDer, LOW);
    ledcWrite(pwmIzq, 0);
    ledcWrite(pwmDer, 0);
  }
}

void Lectura_de_sensores(void *)
{
  TickType_t Ultimo = xTaskGetTickCount();
  for (;;)
  {
    // Se desarrolla la configuración mediante FreeRTOS del sistema para el sensor DHT22
    float Lectura_De_Temperatura_Incubadora = dht1.readTemperature();
    float Lectura_De_Humedad_Incubadora = dht1.readHumidity();
    // Recordar que el UFF del sistema de la temperatura en un sistema incubador posee mayor logica.
    float Lectura_De_Temperatura_Ambiental = dht2.readTemperature();
    portENTER_CRITICAL(&muxSensors);
    if (!isnan(Lectura_De_Humedad_Incubadora))
      Point_Value_Humedad_Incubadora = Lectura_De_Humedad_Incubadora;
    if (!isnan(Lectura_De_Temperatura_Incubadora))
      Point_Value_Temperatura_Incubadora = Lectura_De_Temperatura_Incubadora;
    if (!isnan(Lectura_De_Temperatura_Ambiental))
      Point_Value_Temperatura_Ambiental = Lectura_De_Temperatura_Ambiental;
    portEXIT_CRITICAL(&muxSensors);
    vTaskDelayUntil(&Ultimo, pdMS_TO_TICKS(Tiempo_de_Muestreo));
    return poi
  }
}

void FeedForward()
{
  Lectura_de_sensores();
  portENTER_CRITICAL(&muxSensors);
  Temperatura_Ambiental = point
      // Aqui se realizara la recuperación de los parametros del proceso.
      Feed_Forward_Temperatura = .3 * (Setpoint - 25) / K_Temperatura;
  Feed_Forward_Humedad = .3 * (Setpoint - 25) / K_Humedad;
}

void Calculo_Temperatura()
{
  // Calculo del sistema de temperatura en ecuaciones de diferencias.
  Y0_Temperatura[0] = X0_Temperatura[0] * a0_H1[0] - X0_Temperatura[1] * (Kpd + 2 * Kdd) + X0_Temperatura[2] * Kdd + Y0_Temperatura[1];
  for (int i = 3; i > 0; i--)
  {
    X0_Temperatura[i] = X0_Temperatura[i - 1];
    Y0_Temperatura[i] = Y0_Temperatura[i - 1];
  }
  Configuracion_PID(tiem)
}
void Calculo_Humedad()
{
  Y0_Humedad[0] = X0_Humedad[0] * (Kpd + Kid + Kdd) - X0_Humedad[1] * (Kpd + 2 * Kdd) + X0_Humedad[2] * Kdd + Y0_T1[1];
  for (int i = 3; i > 0; i--)
  {
    X0_Humedad[i] = X0_Humedad[i - 1];
    Y0_Humedad[i] = Y0_Humedad[i - 1];
  }
}

void Calculo_PID_Temperatura()
{
  Calculo_Temperatura();
  Y0_PID_Temperatura[0] = X0_PID_Temperatura[0] * (Kpd + Kid + Kdd) - X0_PID_Temperatura[1] * (Kpd + 2 * Kdd) + X0_PID_Temperatura[2] * Kdd + Y0_PID_Temperatura[1];
  for (int i = 3; i > 0; i--)
  {
    X0_PID_Temperatura[i] = X0_PID_Temperatura[i - 1];
    Y0_PID_Temperatura[i] = Y0_PID_Temperatura[i - 1];
  }
}

void Calculo_PID_Humedad()
{
  Calculo_Humedad();
  Y0_PID_Humedad[0] = X0_PID_Humedad[0] * (Kpd + Kid + Kdd) - X0_PID_Humedad[1] * (Kpd + 2 * Kdd) + X0_PID_Humedad[2] * Kdd + Y0_PID_Humedad[1];
  for (int i = 3; i > 0; i--)
  {
    X0_PID_Humedad[i] = X0_PID_Humedad[i - 1];
    Y0_PID_Humedad[i] = Y0_PID_Humedad[i - 1];
  }
}

void Suma_Total()
{
  Calculo_PID_Humedad();
  Calculo_PID_Temperatura();
}

void setup()
{
  Serial.begin(115200);
  Tiempo_Anterior = micros();
  xTaskCreatePinnedToCore(Lectura_de_sensores, "DHT", 4096, nullptr, 1, nullptr, 0);
  pinMode(Ventilador, OUTPUT);
  pinMode(Calentador_Temperatura, OUTPUT);
  pinMode(Sistema_Dosificador_de_Humedad, OUTPUT);
  pinMode(Sensor_DHT22, INPUT);
  pinMode(Sensor_DHT22_Ambiental, INPUT);
  digitalWrite(Ventilador, HIGH);
  digitalWrite(Calentador_Temperatura, LOW);
  digitalWrite(Sistema_Dosificador_de_Humedad, LOW);

  ledcSetup(0, 5000, 12);
  ledcAttachPin(EnableA, 0);
  ledcSetup(3, 5000, 12);
  ledcAttachPin(EnableA_DER, 3);

  if (!LittleFS.begin(true))
    Serial.println("LittleFS mount falló");

  preferences.begin("wifi", false);
  ssidGuardado = preferences.getString("ssid", "");
  passGuardado = preferences.getString("password", "");
  preferences.end();

  webServer.on("/", manejarRaiz);
  webServer.on("/guardar", HTTP_POST, manejarGuardar);
  webServer.on("/escanear", HTTP_GET, manejarEscanear);
  webServer.onNotFound(manejarArchivo);
  webServer.begin();

  bool ok = false;
  if (ssidGuardado != "")
    ok = conectarWiFi(ssidGuardado, passGuardado);

  if (!ok)
  {
    Serial.println("Portal AP: " + String(AP_SSID));
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASS);
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    while (WiFi.status() != WL_CONNECTED)
    {
      dnsServer.processNextRequest();
      webServer.handleClient();
      delay(10);
    }
    dnsServer.stop();
  }

  Serial.print("WiFi OK · IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Abre: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  Serial.println("Modo simulación disponible vía WS: {type:'set_sim', value:true|false}");

  wsServer.begin();
  wsServer.onEvent(onWsEvent);

  xTaskCreatePinnedToCore(tareaSensores, "DHT", 4096, nullptr, 1, nullptr, 0);

  ultimoPID = millis();
  ultimoWS = millis();
}

void loop()
{
  Tiempo_Actual = micros();
}
