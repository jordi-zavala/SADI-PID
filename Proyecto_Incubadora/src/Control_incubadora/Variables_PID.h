#pragma once
#include "../Configuracion_Esp_32/Variables_del_sistema.h"

// ============================================================
// CONFIGURACIÓN DE ECUACIONES DE DIFERENCIAS
// ============================================================
float a0_T1 = 1 - exp(-Tiempo_de_Muestreo / Tiempo_de_Subida_Sistema_Temperatura); 
float b0_T1 = 0.1; 

float a0_H1 = 1 - exp(-Tiempo_de_Muestreo / Tiempo_de_Subida_Sistema_Humedad);
float b0_H1 = 0.1; 

float Kpd = 0, Kid = 0, Kdd = 0;

float X0_Temperatura[4] = {0, 0, 0, 0};
float Y0_Temperatura[4] = {0, 0, 0, 0};
float X0_Humedad[4] = {0, 0, 0, 0};
float Y0_Humedad[4] = {0, 0, 0, 0};

float X0_PID_Temperatura[4] = {0, 0, 0, 0};
float Y0_PID_Temperatura[4] = {0, 0, 0, 0};
float X0_PID_Humedad[4] = {0, 0, 0, 0};
float Y0_PID_Humedad[4] = {0, 0, 0, 0};

float Offset_Temperatura = 0.0;
float Offset_Humedad = 0.0;
