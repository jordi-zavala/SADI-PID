# Título del Proyecto: Sistema de Control Digital en Lazo Cerrado para Regulación de Temperatura y Humedad en Incubadora

## Objetivo General

Diseñar e implementar un sistema mecatrónico de control climático para una incubadora utilizando un microcontrolador ESP32. El sistema regula dinámicamente la temperatura y la humedad relativa internas mediante actuadores de calefacción y humidificación, garantizando condiciones ambientales estables y controladas para el desarrollo biológico, rechazando perturbaciones térmicas y de carga mediante una arquitectura de control multivariable en lazo cerrado.

## Arquitectura de Control (Estrategia)

El proyecto se fundamenta en un esquema de control multivariable (MIMO 2×2) que combina dos lazos de realimentación acoplados:

- **Cálculo Dinámico de Referencia (Feedforward):** Un sensor de temperatura ambiente exterior y un sensor de apertura de puerta permiten al algoritmo anticipar el efecto de perturbaciones externas sobre la cámara. Esta lectura es procesada matemáticamente para ajustar preventivamente las consignas o las señales de control antes de que el error se manifieste.
- **Regulación en Lazo Cerrado (Feedback):** Dos sensores internos retroalimentan la temperatura y la humedad relativa reales dentro de la cámara. Dos controladores digitales implementados en el ESP32 (un PID discreto para temperatura y un PI discreto para humedad) calculan el error entre la condición real y el *setpoint*, ajustando el ciclo de trabajo de las señales PWM enviadas a las etapas de potencia.
- **Desacoplo (Mejora opcional):** Una red de desacoplo entre las salidas de los controladores y los actuadores compensa el acoplamiento cruzado: el calefactor afecta la humedad relativa y el humidificador puede afectar la temperatura. Esto permite que cada lazo actúe sin perturbar significativamente al otro.

## Hardware y Elementos de la Planta

- **Unidad de Procesamiento:** Microcontrolador ESP32 operando como el controlador digital central, ejecutando ambos lazos de control en tiempo discreto.
- **Sensores:**
  - Sensor de temperatura y humedad interno (SHT31 o DHT22) ubicado en un punto representativo de la cámara, lejos del calefactor y del humidificador.
  - Sensor de temperatura ambiente exterior (DS18B20 o similar) para alimentar el lazo feedforward.
  - Sensor de apertura de puerta (reed switch o interruptor) para detección de perturbaciones bruscas.
- **Actuadores:**
  - Resistencia calefactora controlada por MOSFET de nivel lógico o SSR, con modulación por ancho de pulso (PWM) o time-proportioning.
  - Humidificador ultrasónico controlado por relé o MOSFET.
  - Ventilador de homogeneización controlado por PWM para distribuir el aire sin enfriar en exceso.
- **Planta:** Cámara térmicamente aislada con inercia térmica significativa, dinámica lenta, retardo de transporte y comportamiento no lineal por saturación de los actuadores.

## Justificación Técnica (El valor del Control Digital)

A diferencia de los enfoques convencionales de lazo abierto que simplemente aplican una tabla de valores fijos (temperatura vs. porcentaje de PWM), este diseño rechaza activamente las perturbaciones internas y externas de la planta. El controlador compensa variables como la caída de tensión en la fuente de alimentación, la pérdida de calor por apertura de puerta y el retardo térmico del sistema, asegurando que la temperatura y la humedad reales alcancen y mantengan la referencia calculada de forma rápida, estable y segura. La naturaleza multivariable del sistema exige además gestionar el acoplamiento entre ambos lazos, lo que añade un nivel de complejidad y rigor propio de un proyecto de control digital avanzado.
