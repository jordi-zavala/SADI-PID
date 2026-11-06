#include "DHT.h"

// ==========================================
// CONFIGURACIÓN DE PINES
// ==========================================
#define DHTPIN 4
#define DHTTYPE DHT22

// Pines para el Puente H BTS7960 (Solo un sentido)
#define L_EN 6       // Enable (Habilitador)
#define L_PWM 7      // PWM (Control de potencia)

DHT dht(DHTPIN, DHTTYPE);

// Configuración PWM (Nueva API unificada)
const int freq = 1000;      // 1 kHz
const int resolucion = 12;  // 12 bits (0 a 4095)

// Potencia del escalón (2048 = 50%, 4095 = 100%)
const int potencias_escalon = 3071; 

// Control de tiempos
unsigned long tiempo_inicio;
unsigned long ultimo_registro = 0;
bool experimento_iniciado = false;

// Tiempo máximo de seguridad: 30 minutos (1800000 ms)
const unsigned long TIEMPO_MAXIMO = 1800000; 

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Configurar pines del BTS7960
  pinMode(L_EN, OUTPUT);
  
  // Habilitar el puente H
  digitalWrite(L_EN, HIGH);
  
  // Configurar PWM con la nueva API (reemplaza ledcSetup y ledcAttachPin)
  ledcAttach(L_PWM, freq, resolucion);
  ledcWrite(L_PWM, 0); // Empezar apagado
  
  Serial.println("Esperando 30s para estabilizar temperatura ambiente...");
  delay(10000); 
  
  // Cabecera del CSV
  Serial.println("Tiempo_s,Temperatura_C,Humedad_Rel");
  
  tiempo_inicio = millis();
  ultimo_registro = tiempo_inicio;
  experimento_iniciado = true;
  
  // ¡ESCALÓN! Encender el foco
  ledcWrite(L_PWM, potencias_escalon);
}

void loop() {
  if (experimento_iniciado) {
    unsigned long tiempo_actual = millis();
    
    // Leer cada 2000 milisegundos (2 segundos)
    if (tiempo_actual - ultimo_registro >= 2000) {
      ultimo_registro = tiempo_actual;
      
      float tiempo_s = (tiempo_actual - tiempo_inicio) / 1000.0;
      float temp = dht.readTemperature();
      float hum = dht.readHumidity();
      
      if (!isnan(temp) && !isnan(hum)) {
        Serial.print(tiempo_s);
        Serial.print(",");
        Serial.print(temp);
        Serial.print(",");
        Serial.println(hum);
      }
    }
    
    // Condición de parada (30 minutos)
    if (tiempo_actual - tiempo_inicio > TIEMPO_MAXIMO) {
      ledcWrite(L_PWM, 0); // Apagar foco
      digitalWrite(L_EN, LOW); // Deshabilitar BTS7960 por seguridad
      Serial.println("Tiempo máximo alcanzado. Calentador apagado.");
      experimento_iniciado = false;
    }
  }
}