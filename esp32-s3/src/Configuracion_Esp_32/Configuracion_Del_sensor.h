#pragma once
#include <DHT.h>
#include "Pines_Incubadora.h"
#include "Freertos.h"
#include "Variables_del_sistema.h"

// Creacion de identificadores:
DHT dht1(Sensor_DHT22_Sistema, TIPO_DHT);
DHT dht2(Sensor_DHT22_Ambiental, TIPO_DHT);

volatile float Point_Value_Humedad_Incubadora = NAN, Point_Value_Temperatura_Incubadora = NAN, Point_Value_Temperatura_Ambiental = NAN;
portMUX_TYPE muxSensors = portMUX_INITIALIZER_UNLOCKED;

// ============================================================
// TAREA DE SENSORES
// ============================================================
void Lectura_de_sensores(void *)
{
  TickType_t Ultimo = xTaskGetTickCount();
  for (;;)
  {
    float Lectura_De_Temperatura_Incubadora = dht1.readTemperature();
    float Lectura_De_Humedad_Incubadora = dht1.readHumidity();
    float Lectura_De_Temperatura_Ambiental = dht2.readTemperature();
    
    portENTER_CRITICAL(&muxSensors);
    if (!isnan(Lectura_De_Humedad_Incubadora)) Point_Value_Humedad_Incubadora = Lectura_De_Humedad_Incubadora;
    if (!isnan(Lectura_De_Temperatura_Incubadora)) Point_Value_Temperatura_Incubadora = Lectura_De_Temperatura_Incubadora;
    if (!isnan(Lectura_De_Temperatura_Ambiental)) Point_Value_Temperatura_Ambiental = Lectura_De_Temperatura_Ambiental;
    portEXIT_CRITICAL(&muxSensors);
    
    vTaskDelayUntil(&Ultimo, pdMS_TO_TICKS(Tiempo_de_Muestreo * 1000)); // FIX: segundos -> ms (antes 2 ms)
  }
}
