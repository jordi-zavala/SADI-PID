#include <Arduino.h>
#include <math.h>

#include "Configuracion_Esp_32/Freertos.h"
#include "Configuracion_Esp_32/Pines_Incubadora.h"
#include "Configuracion_Esp_32/Variables_del_sistema.h"
#include "Configuracion_Esp_32/Configuracion_Del_sensor.h"
#include "Configuracion_Esp_32/Configuracion_Wifi.h"
#include "Control_incubadora/Variables_PID.h"
#include "Control_incubadora/Calculo_del_Sistema.h"

unsigned long Tiempo_Actual, Tiempo_Anterior;

// ============================================================
// SETUP Y LOOP PRINCIPAL
// ============================================================
void setup()
{
  Serial.begin(115200);
  Tiempo_Anterior = micros();
  
  pinMode(EN_Calentador, OUTPUT);
  pinMode(RPWM_Calentador, OUTPUT);
  pinMode(IN2_Humedad, OUTPUT);
  pinMode(Dosificador_de_Humedad_EN_A, OUTPUT);
  pinMode(IN4_Ventilador, OUTPUT);
  pinMode(Ventilador_EN_B, OUTPUT);
  pinMode(Sensor_DHT22_Sistema, INPUT);
  pinMode(Sensor_DHT22_Ambiental, INPUT);
  
  digitalWrite(EN_Calentador, HIGH);     // habilita el BTS7960 (potencia = 0 hasta que el PID mande)
  digitalWrite(IN4_Ventilador, HIGH);    // ventilador encendido (L298N canal B, IN3 a GND)
  digitalWrite(Ventilador_EN_B, HIGH);
  digitalWrite(IN2_Humedad, LOW);
  digitalWrite(Dosificador_de_Humedad_EN_A, LOW);

  dht1.begin(); // FIX: faltaba inicializar los sensores
  dht2.begin();

  // FIX: Arduino-ESP32 3.x configura el PWM por pin (ledcSetup/ledcAttachPin ya no existen)
  ledcAttach(RPWM_Calentador, FREQ, RESOLUTION);
  ledcAttach(Dosificador_de_Humedad_EN_A, FREQ, RESOLUTION);

  Configuracion_PID(Tiempo_de_Subida_Sistema_Temperatura, Tiempo_de_Retardo_Sistema_Temperatura, Tiempo_de_Retardo_Sensor_Temperatura, K_Temperatura);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount falló");
  }

  preferences.begin("wifi", false);
  Nombre_Wifi_Guardado = preferences.getString("ssid", "");
  Password_Wifi_Guardado = preferences.getString("password", "");
  preferences.end();

  // ====== ENRUTAMIENTO WEB ======
  webServer.on("/", manejarRaiz);
  webServer.on("/guardar", HTTP_POST, manejarGuardar);
  webServer.on("/escanear", HTTP_GET, manejarEscanear);
  webServer.onNotFound(manejarArchivo);
  // FIX: webServer.begin() se movió después de iniciar el WiFi (abre un socket y la pila de red aún no existía)

  // ====== CONEXIÓN WIFI ======
  bool ok = false;
  if (Nombre_Wifi_Guardado != "") {
    WiFi.begin(Nombre_Wifi_Guardado.c_str(), Password_Wifi_Guardado.c_str());
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) { // 10 segundos max
      delay(500);
      Serial.print(".");
      intentos++;
    }
    if(WiFi.status() == WL_CONNECTED) ok = true;
  }

  // Si falla la conexión, levanta el portal AP para configurar
  if (!ok) {
    Serial.println("Portal AP: " + String(AP_SSID));
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASS);
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  }

  Serial.print("WiFi listo. IP: ");
  Serial.println(WiFi.localIP());

  // ====== INICIO SERVIDORES, WEBSOCKET Y TAREAS ======
  webServer.begin();
  wsServer.begin();
  wsServer.onEvent(onWsEvent);
  xTaskCreatePinnedToCore(Lectura_de_sensores, "DHT", 4096, nullptr, 1, nullptr, 0);
}

void loop()
{
  // Procesar peticiones del servidor DNS (si está en modo AP)
  dnsServer.processNextRequest();
  
  // Procesar peticiones HTTP (WiFi y archivos)
  webServer.handleClient();
  
  // Procesar peticiones WebSocket (Scope)
  wsServer.loop();
  
  Tiempo_Actual = micros();
  
  // Ejecutar control cada 2 segundos
  if (Tiempo_Actual - Tiempo_Anterior >= Tiempo_de_Muestreo * 1000000) 
  {
    Tiempo_Anterior = Tiempo_Actual;

    FeedForward();
    
    if (!isnan(Point_Value_Temperatura_Incubadora) && !isnan(Point_Value_Humedad_Incubadora))
    {
      Calculo_PID_Temperatura();
      Calculo_PID_Humedad();
      
      int Salida_Calentador = constrain((int)Y0_PID_Temperatura[0], 0, 255);
      int Salida_Humedad = constrain((int)Y0_PID_Humedad[0], 0, 255);
      
      Controlar_Calentador_BTS7960(Salida_Calentador);
      Controlar_Humedificador_L298N(Salida_Humedad);
    } else {
      Controlar_Calentador_BTS7960(0);
      Controlar_Humedificador_L298N(0);
    }
  }

  // Enviar datos al Scope por WebSocket
  enviarDatosScope();
}
