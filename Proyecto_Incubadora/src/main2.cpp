#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WebSocketsServer.h>
#include "esp_adc_cal.h"


const int sensorPin = 34;
float relacionDivisor = 4.93;
float porcentajeSuavizado = 100.0;
int porcentajeEntero = 100;
const float alpha = 0.10;

esp_adc_cal_characteristics_t adc_chars;
WebSocketsServer webSocket(81);

float calcularPorcentaje(float v) {
  if (v >= 12.60) return 100;
  if (v <= 10.50) return 0;
  return (v - 10.50) * 100.0 / (12.60 - 10.50);
}

float medirVoltaje() {
  uint32_t suma = 0;
  for (int i = 0; i < 32; i++) {
    suma += analogRead(sensorPin);
    delayMicroseconds(200);
  }
  float vPin = esp_adc_cal_raw_to_voltage(suma / 32, &adc_chars) / 1000.0;
  return vPin * relacionDivisor;
}

void setup() {
  Serial.begin(115200);


  analogReadResolution(12);
  analogSetPinAttenuation(sensorPin, ADC_11db);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);


  WiFiManager wm;
  // wm.resetSettings(); //Si no sale el wifi descomentas el codigo y ya que agarre lo vuelves a comentar :v
  bool conectado = wm.autoConnect("CleanTECH", "12345678");

  if (!conectado) {
    Serial.println("No se pudo conectar. Reiniciando...");
    delay(2000);
    ESP.restart();
  }

  Serial.println("Conectado a WiFi");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());


  webSocket.begin();
}

void loop() {
  webSocket.loop();

  float vBat = medirVoltaje();
  float pct = calcularPorcentaje(vBat);

  porcentajeSuavizado = alpha * pct + (1 - alpha) * porcentajeSuavizado;
  int entero = round(porcentajeSuavizado);

  if (entero < porcentajeEntero) {
    porcentajeEntero = entero;
  }

  Serial.print(vBat, 2);
  Serial.print(" V | ");
  Serial.print(porcentajeEntero);
  Serial.println(" %");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  String msg = "{\"voltaje\":" + String(vBat, 2) +
                ",\"porcentaje\":" + String(porcentajeEntero) + "}";
  webSocket.broadcastTXT(msg);

  delay(1000);
}
