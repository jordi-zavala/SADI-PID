# Título del Proyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa (SADI)

## Objetivo General

Diseñar e implementar un sistema de control digital en lazo cerrado para regular la temperatura y la humedad relativa dentro de una incubadora, utilizando un microcontrolador ESP32 como controlador central, con el fin de mantener condiciones ambientales estables y rechazar perturbaciones externas.

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

## Descripción general del prototipo SADI.

El proyecto *SADI (Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa)* se concibe como una solución integral que reemplaza la conmutación manual por un lazo cerrado de control digital. Su objetivo fundamental es regular de manera autónoma el microclima de una cámara de incubación, rechazando perturbaciones externas (tales como la pérdida repentina de calor por la apertura de compuertas) y estabilizando la planta térmica. El sistema fue diseñado con una meta de precisión estacionaria de ±1°C para la variable de temperatura y de ±10% para la humedad relativa.

![Diagrama de bloques para el control del sistema SADI.](img/diagrama_bloques_sadi.webp)

Físicamente, el prototipo es una planta multivariable que integra:

- **Elementos de actuación:** Un foco halógeno resistivo como fuente térmica principal y un módulo ultrasónico sumergible para la inyección de humedad. Ambos elementos están apoyados por un ventilador axial de corriente continua encargado de generar convección forzada para homogeneizar el volumen de aire.
- **Sistema de instrumentación:** La retroalimentación de la planta se obtiene en tiempo real mediante un sensor digital DHT22, el cual monitorea las condiciones térmicas e higrométricas del ambiente interno.

## Arquitectura de control digital.

Para cumplir con los requerimientos dinámicos del sistema, la lógica de control recae sobre un microcontrolador ESP32-S3. Este dispositivo actúa como la unidad de procesamiento central, encargado de ejecutar los algoritmos PID discretizados a un periodo de muestreo ($T_s$) constante @astrom_pid.

La arquitectura electrónica incluye una etapa de potencia basada en un módulo puente H de dos salidas. Esta configuración permite al controlador aislar las corrientes de demanda operativa y enviar señales de control moduladas por ancho de pulso (PWM). De esta manera, el algoritmo PID calcula el esfuerzo de control en tiempo real (basándose en la magnitud, la acumulación y la tasa de cambio del error) y la etapa de potencia traduce ese valor matemático en una modulación proporcional de la energía eléctrica entregada al foco calefactor y al humidificador ultrasónico.