#pragma once
// NOTA: los headers definen variables globales; inclúyelos solo desde main.cpp.
#include <Arduino.h>
#include <math.h>

String usuarios_Permitidos[20];
String Nombre_Wifi_Guardado = "";
String Password_Wifi_Guardado = "";

// Configuración Puente H
constexpr int FREQ = 15000, RESOLUTION = 8;
constexpr int CH_RPWM = 0, CH_LPWM = 1, CH_HUMEDAD = 3;

float Feed_Forward_Temperatura, Feed_Forward_Humedad, Setpoint = 37.5, Setpoint_Humedad = 60.0, Temperatura_Ambiental;

// Configuración de los tiempos del sistema.
constexpr double Tiempo_Maximo_Ejecucion = 1.2e3, Tiempo_de_Muestreo = 2;
constexpr double Tiempo_de_Subida_Sistema = 172.0;
constexpr double Tiempo_de_Subida_Sistema_Temperatura = 172.0,
                 Tiempo_de_Retardo_Sistema_Temperatura = 4,
                 Tiempo_de_Subida_Sensor_Temperatura = 26.38,
                 Tiempo_de_Retardo_Sensor_Temperatura = 4.2;

constexpr double Tiempo_de_Subida_Sistema_Humedad = 100.0,
                 Tiempo_de_Retardo_Sistema_Humedad = 4.2,
                 Tiempo_de_Subida_Sensor_Humedad = 26.38,
                 Tiempo_de_Retardo_Sensor_Humedad = 5.0;

constexpr double K_Temperatura = 68.50, K_Humedad = 40;
