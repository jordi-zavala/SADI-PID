#pragma once
#include <Arduino.h>
#include "../Configuracion_Esp_32/Pines_Incubadora.h"
#include "../Configuracion_Esp_32/Variables_del_sistema.h"
#include "../Configuracion_Esp_32/Configuracion_Del_sensor.h"
#include "Variables_PID.h"

// ============================================================
// CONTROL DE ACTUADORES (BTS7960 y L298N)
// ============================================================
void Controlar_Calentador_BTS7960(int velocidad) {
  ledcWrite(RPWM_Calentador, velocidad); // FIX: 3.x escribe por pin 
}

void Controlar_Humedificador_L298N(int velocidad) {
  digitalWrite(IN2_Humedad, HIGH); // IN1 del L298N va a GND
  ledcWrite(Dosificador_de_Humedad_EN_A, velocidad);
}

// ============================================================
// FEED-FORWARD Y CÁLCULO DE OFFSET DINÁMICO
// ============================================================
void FeedForward()
{
  portENTER_CRITICAL(&muxSensors);
  Temperatura_Ambiental = Point_Value_Temperatura_Ambiental;
  portEXIT_CRITICAL(&muxSensors);

  Feed_Forward_Temperatura = 0.3 * (Setpoint - Temperatura_Ambiental) / K_Temperatura;
  Feed_Forward_Humedad = 0.3 * (Setpoint_Humedad - Temperatura_Ambiental) / K_Humedad;

  Offset_Temperatura = (1.0 - a0_T1) * Temperatura_Ambiental - b0_T1 * Feed_Forward_Temperatura;
  Offset_Humedad = (1.0 - a0_H1) * 50.0 - b0_H1 * Feed_Forward_Humedad;
}

// ============================================================
// ECUACIONES DE DIFERENCIAS Y PID
// ============================================================
void Calculo_Temperatura()
{
  Y0_Temperatura[0] = Y0_Temperatura[1] * a0_T1 + X0_Temperatura[1] * b0_T1 + Offset_Temperatura;
  for (int i = 3; i > 0; i--) {
    X0_Temperatura[i] = X0_Temperatura[i - 1];
    Y0_Temperatura[i] = Y0_Temperatura[i - 1];
  }
}

void Calculo_Humedad()
{
  Y0_Humedad[0] = Y0_Humedad[1] * a0_H1 + X0_Humedad[1] * b0_H1 + Offset_Humedad;
  for (int i = 3; i > 0; i--) {
    X0_Humedad[i] = X0_Humedad[i - 1];
    Y0_Humedad[i] = Y0_Humedad[i - 1];
  }
}

void Calculo_PID_Temperatura()
{
  Calculo_Temperatura();
  X0_PID_Temperatura[0] = Setpoint - Point_Value_Temperatura_Incubadora;
  Y0_PID_Temperatura[0] = Y0_PID_Temperatura[1] + 
                          X0_PID_Temperatura[0] * (Kpd + Kid + Kdd) - 
                          X0_PID_Temperatura[1] * (Kpd + 2 * Kdd) + 
                          X0_PID_Temperatura[2] * Kdd;
  for (int i = 3; i > 0; i--) {
    X0_PID_Temperatura[i] = X0_PID_Temperatura[i - 1];
    Y0_PID_Temperatura[i] = Y0_PID_Temperatura[i - 1];
  }
}

void Calculo_PID_Humedad()
{
  Calculo_Humedad();
  X0_PID_Humedad[0] = Setpoint_Humedad - Point_Value_Humedad_Incubadora;
  Y0_PID_Humedad[0] = Y0_PID_Humedad[1] + 
                      X0_PID_Humedad[0] * (Kpd + Kid + Kdd) - 
                      X0_PID_Humedad[1] * (Kpd + 2 * Kdd) + 
                      X0_PID_Humedad[2] * Kdd;
  for (int i = 3; i > 0; i--) {
    X0_PID_Humedad[i] = X0_PID_Humedad[i - 1];
    Y0_PID_Humedad[i] = Y0_PID_Humedad[i - 1];
  }
}

void Configuracion_PID(float Tau, float Retardo_sys, float Retardo_Sensor, float Gananica_Planta)
{
  const double Kp = (Tau * 0.6) / (Gananica_Planta * (Retardo_Sensor + Retardo_sys + Tiempo_de_Muestreo) * 2.0), // FIX: constexpr -> const (usa parámetros en ejecución)
               Ti = Tiempo_de_Subida_Sistema,
               Td = 0.0;
  Kid = (Kp * Tiempo_de_Muestreo) / Ti;
  Kdd = (Kp * Td) / Tiempo_de_Muestreo;
  Kpd = Kp;
}
