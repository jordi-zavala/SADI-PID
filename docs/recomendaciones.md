# Revisión del proyecto SADI: fallos y recomendaciones

Revisión del firmware en [`esp32-s3/src/`](../esp32-s3/src/), con foco en el control PID de temperatura y humedad. Los puntos están ordenados por prioridad. En cada uno se indica el archivo y la línea, y se propone una corrección.

**Leyenda:** 🔴 impide que el control funcione · 🟠 fallo de diseño o seguridad · 🟡 mejora recomendada

---

## Resumen

| # | Hallazgo | Prioridad |
|---|---|---|
| 1 | La salida del PID no está escalada a PWM: el calentador casi no enciende | 🔴 |
| 2 | Humedad usa las mismas ganancias que temperatura | 🔴 |
| 3 | Sin anti-windup: el estado del PID crece sin límite | 🔴 |
| 4 | El feedforward se calcula pero nunca se aplica | 🔴 |
| 5 | Feedforward de humedad mezcla %HR con °C | 🟠 |
| 6 | Si el sensor falla, el PID sigue con la última lectura (riesgo de sobrecalentar) | 🟠 |
| 7 | Sin corte de seguridad por sobretemperatura | 🟠 |
| 8 | El periodo de muestreo no es constante (jitter por el servidor web) | 🟠 |
| 9 | Sensor y control no están sincronizados (retardo extra de hasta 2 s) | 🟡 |
| 10 | Regla de sintonía: `Ti` = tiempo de subida, `Td` = 0 | 🟡 |
| 11 | Modelo de la planta (`Calculo_Temperatura`) es código muerto | 🟡 |
| 12 | Humidificador ultrasónico con PWM de 15 kHz | 🟡 |
| 13 | Acoplamiento temperatura ↔ humedad sin tratar | 🟡 |
| 14 | HMI: Scope incompleto y sin forma de cambiar setpoint, ganancias ni modo | 🟡 |
| 15 | Detalles de WiFi y código | 🟡 |

---

## 🔴 1. La salida del PID no está escalada a PWM

[`Calculo_del_Sistema.h:87`](../esp32-s3/src/Control_incubadora/Calculo_del_Sistema.h#L87) y [`main.cpp:118`](../esp32-s3/src/main.cpp#L118)

Con los valores actuales de [`Variables_del_sistema.h`](../esp32-s3/src/Configuracion_Esp_32/Variables_del_sistema.h):

```
Kp  = 0.6·172 / (68.5 · (4 + 4.2 + 2) · 2) ≈ 0.074
Kid = Kp·Ts/Ti = 0.074·2/172               ≈ 0.00086
```

La salida se manda directo a `ledcWrite()`, que con 8 bits va de **0 a 255**. Con 10 °C de error, la parte proporcional da **0.74 cuentas**, menos del 0.3 % del PWM. La integral sube 0.0086 cuentas por muestra, así que tardaría horas en llegar a un valor útil. **En la práctica el calentador no enciende.**

El feedforward usa la misma escala: `0.3·(37.5−24)/68.5 ≈ 0.06`. Eso sugiere que la ganancia `K_Temperatura = 68.5` está en **°C por unidad de duty (0–1)**. Si es así, el PID entrega un duty de 0 a 1 que hay que convertir a cuentas:

```cpp
// u en [0, 1]  ->  cuentas PWM [0, 255]
constexpr int PWM_MAX = (1 << RESOLUTION) - 1;
int Salida_Calentador = constrain((int)(u_temp * PWM_MAX), 0, PWM_MAX);
```

**Qué hacer:** confirmar en qué unidades se identificó `K` (°C/%duty, °C/cuenta o °C/duty 0–1) y usar las mismas en el PID, el feedforward y la saturación. Conviene anotarlo junto a la constante.

## 🔴 2. Humedad usa las ganancias de temperatura

[`Variables_PID.h:13`](../esp32-s3/src/Control_incubadora/Variables_PID.h#L13), [`main.cpp:44`](../esp32-s3/src/main.cpp#L44)

Solo existe un juego de ganancias, `Kpd`, `Kid` y `Kdd`. `Configuracion_PID()` se llama una sola vez, con los parámetros de temperatura. `Calculo_PID_Humedad()` usa esas mismas ganancias, así que la humedad nunca se sintoniza con sus propios parámetros (`K_Humedad`, `Tiempo_de_Subida_Sistema_Humedad`, etc.).

**Corrección:** usar una estructura por lazo:

```cpp
struct PID_Discreto {
  float Kp, Ki, Kd;      // ganancias discretas
  float e[3] = {0};      // e[k], e[k-1], e[k-2]
  float u = 0;           // u[k-1]
  float u_min, u_max;    // saturación del actuador
};

PID_Discreto pid_T, pid_H;
Configuracion_PID(pid_T, Tau_T, Retardo_T, Retardo_Sensor_T, K_Temperatura);
Configuracion_PID(pid_H, Tau_H, Retardo_H, Retardo_Sensor_H, K_Humedad);
```

Así se elimina además el código duplicado entre `Calculo_PID_Temperatura` y `Calculo_PID_Humedad`.

## 🔴 3. Sin anti-windup en el PID en forma de velocidad

[`Calculo_del_Sistema.h:87-94`](../esp32-s3/src/Control_incubadora/Calculo_del_Sistema.h#L87-L94)

El algoritmo es el PID incremental (forma de velocidad):

$$u[k] = u[k-1] + (K_p + K_i + K_d)\,e[k] - (K_p + 2K_d)\,e[k-1] + K_d\,e[k-2]$$

La ecuación está bien. El problema es que `constrain()` se aplica solo al valor que va al PWM ([`main.cpp:118`](../esp32-s3/src/main.cpp#L118)), mientras `Y0_PID_Temperatura` guarda el valor **sin saturar**. Durante el calentamiento inicial `u` puede llegar a valores muy por encima de 255. Cuando la temperatura pasa el setpoint, el controlador tarda mucho en bajar: es *windup* y produce un sobreimpulso grande.

En la forma de velocidad el anti-windup es sencillo: basta con **saturar `u[k]` antes de guardarlo** como `u[k-1]`.

```cpp
float u = pid.u + (pid.Kp + pid.Ki + pid.Kd) * e0
                - (pid.Kp + 2 * pid.Kd) * pid.e[1]
                + pid.Kd * pid.e[2];
u = constrain(u, pid.u_min, pid.u_max);   // <- anti-windup
pid.u = u;
```

Para el reporte vale la pena mostrar las dos respuestas, con y sin anti-windup, ante el escalón de arranque (de temperatura ambiente a 37.5 °C).

## 🔴 4. El feedforward no se aplica

[`Calculo_del_Sistema.h:49-60`](../esp32-s3/src/Control_incubadora/Calculo_del_Sistema.h#L49-L60)

`FeedForward()` calcula `Feed_Forward_Temperatura` y `Feed_Forward_Humedad`, pero esos valores solo alimentan `Offset_*`. `Offset_*` a su vez solo se usa en `Calculo_Temperatura()`/`Calculo_Humedad()`, cuya salida (`Y0_Temperatura`) no se lee en ningún lado (ver punto 11). **El feedforward no llega al actuador**, aunque el README lo presenta como parte de la arquitectura.

Un feedforward estático por temperatura ambiente se suma a la salida del PID:

$$u_{total}[k] = u_{PID}[k] + u_{ff}[k], \qquad u_{ff} = \frac{SP - T_{amb}}{K}$$

`u_ff` es el duty que, en régimen permanente, mantiene la cámara en el setpoint sin error. La saturación y el anti-windup se aplican sobre `u_total`. El factor `0.3` del código actual puede quedar como ajuste fino, pero conviene justificarlo en el reporte.

Si se implementa el sensor de puerta del README, se puede sumar un término extra mientras la puerta esté abierta. Otra opción es congelar la integral (*conditional integration*) para no acumular error durante la apertura.

## 🟠 5. Feedforward de humedad con unidades mezcladas

[`Calculo_del_Sistema.h:56`](../esp32-s3/src/Control_incubadora/Calculo_del_Sistema.h#L56)

```cpp
Feed_Forward_Humedad = 0.3 * (Setpoint_Humedad - Temperatura_Ambiental) / K_Humedad;
```

Resta **%HR − °C**. Debería usar la humedad ambiente, pero el DHT22 ambiental solo lee temperatura ([`Configuracion_Del_sensor.h:21`](../esp32-s3/src/Configuracion_Esp_32/Configuracion_Del_sensor.h#L21)). Hay dos opciones: leer también `dht2.readHumidity()` o quitar el feedforward de humedad. Además, [línea 59](../esp32-s3/src/Control_incubadora/Calculo_del_Sistema.h#L59) usa `50.0` fijo como humedad ambiente.

## 🟠 6. Si el sensor falla, el control sigue con un valor viejo

[`Configuracion_Del_sensor.h:25-27`](../esp32-s3/src/Configuracion_Esp_32/Configuracion_Del_sensor.h#L25-L27)

La tarea del sensor solo actualiza la variable cuando la lectura no es `NaN`. Si el DHT22 se desconecta o falla, la variable conserva **la última lectura válida para siempre**. `main.cpp` solo apaga actuadores si el valor es `NaN` ([`main.cpp:113`](../esp32-s3/src/main.cpp#L113)), y eso solo pasa antes de la primera lectura. Si el sensor falla cuando la cámara está fría, el PID ve un error constante y **mantiene el calentador al máximo indefinidamente**.

**Corrección:** guardar el instante de la última lectura válida y tratar como fallo cualquier dato con más de N muestras de antigüedad:

```cpp
volatile uint32_t Ultima_Lectura_Valida_ms = 0;
// en la tarea, cuando la lectura es válida:
Ultima_Lectura_Valida_ms = millis();
// en el control:
bool sensor_ok = (millis() - Ultima_Lectura_Valida_ms) < 3 * Tiempo_de_Muestreo * 1000;
if (!sensor_ok) { apagar_actuadores(); reiniciar_pid(); }
```

También se puede filtrar lecturas fuera de rango físico (ej. T < 0 °C o > 60 °C) antes de aceptarlas.

## 🟠 7. Sin corte de seguridad por sobretemperatura

No existe un límite duro que apague el foco halógeno de forma independiente del PID. En una incubadora, pasar de ~39.5 °C daña los huevos. Además, un PID mal sintonizado o con windup puede provocar justo eso.

**Recomendación:**
- Agregar un corte por software: si T > SP + 2 °C, calentador a 0, sin importar lo que diga el PID. Debe tener histéresis para volver a habilitarlo.
- Idealmente también uno por hardware: un termostato bimetálico (tipo KSD301) en serie con el foco. Así queda protegido aunque el ESP32 se cuelgue.
- Habilitar el *watchdog* de tareas (`esp_task_wdt`) para que un bloqueo reinicie el sistema.

Esto suma puntos en la sección de seguridad del reporte.

## 🟠 8. Periodo de muestreo no constante

[`main.cpp:93-131`](../esp32-s3/src/main.cpp#L93-L131)

El control se ejecuta dentro de `loop()`, comparando `micros()`. Lo comparte con `webServer.handleClient()`, `wsServer.loop()` y `dnsServer`. Cualquier petición lenta retrasa la muestra:
- `/escanear` llama a `WiFi.scanNetworks()`, que **bloquea 2–4 s** ([`Configuracion_Wifi.h:33`](../esp32-s3/src/Configuracion_Esp_32/Configuracion_Wifi.h#L33)).
- `/guardar` tiene un `delay(1000)`.

Las ganancias discretas (`Kid = Kp·Ts/Ti`, `Kdd = Kp·Td/Ts`) **suponen que `Ts` es exactamente 2 s**. Además, como `Tiempo_Anterior = Tiempo_Actual`, cada retraso se acumula en lugar de compensarse.

**Corrección:** mover el control a su propia tarea de FreeRTOS con `vTaskDelayUntil`, como ya se hace con el sensor, y con prioridad mayor que la de la red:

```cpp
void Tarea_Control(void*) {
  TickType_t t0 = xTaskGetTickCount();
  for (;;) {
    leer_sensores();      // ver punto 9
    calcular_control();
    aplicar_actuadores();
    vTaskDelayUntil(&t0, pdMS_TO_TICKS(Tiempo_de_Muestreo * 1000));
  }
}
xTaskCreatePinnedToCore(Tarea_Control, "Control", 4096, nullptr, 3, nullptr, 1);
```

Para el reporte se puede medir el `Ts` real con `micros()` y graficar su dispersión. Es una buena forma de justificar el uso de un RTOS.

## 🟡 9. Sensor y control desfasados

El sensor se lee en una tarea cada 2 s y el control corre en `loop()` cada 2 s. Las dos fases son independientes, así que el dato que usa el PID puede tener hasta ~2 s de antigüedad. Eso es **retardo extra** que no está en el modelo. Al leer el sensor dentro de la misma tarea de control, justo antes de calcular, se tiene `y[k]` y `u[k]` en el mismo instante, que es lo que supone la teoría de muestreo.

El DHT22 no permite muestrear más rápido que 2 s, así que `Ts = 2 s` es el mínimo. Frente a τ ≈ 172 s está holgado (τ/Ts ≈ 86).

## 🟡 10. Regla de sintonía

[`Calculo_del_Sistema.h:111-119`](../esp32-s3/src/Control_incubadora/Calculo_del_Sistema.h#L111-L119)

- `Ti = Tiempo_de_Subida_Sistema`: usa una variable global y **no el parámetro `Tau`** que recibe la función. Además, Ti = τ no corresponde a ninguna regla clásica para un PI.
- `Td = 0`: es un PI. El README dice "PID discreto para temperatura y PI para humedad".
- El nombre "tiempo de subida" (`Tiempo_de_Subida_*`) se confunde con la constante de tiempo τ. El tiempo de subida 10–90 % es ≈ 2.2τ. Hay que verificar cuál de los dos es el 172 s identificado.
- Typo: `Gananica_Planta`.

**Sugerencia:** usar una regla documentada para modelo FOPDT (primer orden + retardo) e incluir el retardo de muestreo/ZOH (≈ Ts/2) en θ. Por ejemplo **SIMC** (Skogestad), con θ = 4 + 4.2 + Ts/2 = 9.2 s y τc = θ:

$$K_p = \frac{\tau}{K(\tau_c + \theta)} \approx \frac{172}{68.5 \cdot 18.4} \approx 0.136, \qquad T_i = \min(\tau,\ 4(\tau_c+\theta)) \approx 73.6\ \text{s}$$

Se puede comparar con Ziegler-Nichols de lazo abierto y con Cohen-Coon en el reporte.

**Si se agrega la acción derivativa:** el DHT22 tiene resolución de 0.1 °C. Ese escalonamiento hace que `Kd·(e[k]−e[k−1])/Ts` dé picos. Hay que filtrar la derivada (N ≈ 5–10) y aplicarla **solo sobre la medición**, no sobre el error, para evitar el *derivative kick* al cambiar el setpoint.

## 🟡 11. Modelo de la planta sin uso

[`Variables_PID.h:7-11`](../esp32-s3/src/Control_incubadora/Variables_PID.h#L7-L11), [`Calculo_del_Sistema.h:65-81`](../esp32-s3/src/Control_incubadora/Calculo_del_Sistema.h#L65-L81)

`Calculo_Temperatura()` implementa la ecuación de diferencias de la planta, `y[k] = a·y[k−1] + b·u[k−1] + offset`, pero:
- `X0_Temperatura` (la entrada `u`) **nunca se asigna**: siempre vale 0.
- `b0_T1 = 0.1` está fijo. Para una planta FOPDT discretizada con ZOH, `b = K·(1 − e^{−Ts/τ})` ≈ 68.5 · 0.0116 ≈ 0.79.
- Hay una inconsistencia de nombres: en el código `a0_T1 = 1 − e^{−Ts/τ}`, pero en la ecuación se usa como el polo `a`, que debería ser `e^{−Ts/τ}`.
- `Y0_Temperatura` no se usa en ningún lado.

Si la idea era un **observador** o un **predictor de Smith** (que tiene sentido por el retardo de ~10 s), hay que conectarlo. Si no, conviene quitarlo del firmware y llevar ese modelo a MATLAB, donde sí sirve para simular (ver "Para el reporte").

## 🟡 12. Actuador de humedad

[`Calculo_del_Sistema.h:41-44`](../esp32-s3/src/Control_incubadora/Calculo_del_Sistema.h#L41-L44)

Un humidificador ultrasónico tiene su propio oscilador (~1.7 MHz) y normalmente funciona **encendido/apagado**. Alimentarlo con PWM de 15 kHz a través del L298N puede hacer que no arranque, o que el nebulizado no sea proporcional al duty. Opciones:
- **Time-proportioning:** ventana lenta (ej. 10 s). El PID decide el % de la ventana que el humidificador está encendido. Es lo que sugiere el README.
- **On/off con histéresis** (ej. ±3 %HR) en lugar de PI. Es válido y fácil de justificar por la dinámica no lineal del actuador.

Vale la pena hacer una prueba en lazo abierto con varios duty y medir si la respuesta en %HR es realmente proporcional.

El foco halógeno sí tolera bien el PWM a 15 kHz. Por su inercia térmica, una ventana lenta también funcionaría y reduciría el ruido EMI.

## 🟡 13. Acoplamiento entre lazos

El calentador **baja la humedad relativa**: misma humedad absoluta a mayor temperatura da menor %HR. El humidificador, por su parte, enfría un poco la cámara. El README propone una red de desacoplo como mejora opcional. Un orden práctico:
1. Sintonizar temperatura primero, porque es el lazo dominante.
2. Hacer el lazo de humedad más lento (menor ancho de banda) para que vea la temperatura como casi constante.
3. Opcional: desacoplo estático, `u_H += D·u_T`, con `D` medido en lazo abierto.

El RGA (*Relative Gain Array*) de la matriz de ganancias 2×2 justifica en el reporte el emparejamiento de variables.

## 🟡 14. HMI / Scope

- [`Interfaz_Scope.ts`](../esp32-s3/data/Scope/Interfaz_Scope.ts) está incompleto: usa `color`, que no está definida, y nunca abre el WebSocket. La gráfica no funciona ni con la placa. Hay una versión completa anterior en [`include/main.txt:251`](../esp32-s3/include/main.txt#L251).
- `ctrl` envía `Y0_PID_Temperatura[0]` sin saturar ([`Configuracion_Wifi.h:119`](../esp32-s3/src/Configuracion_Esp_32/Configuracion_Wifi.h#L119)). Conviene enviar el valor **aplicado** (en % de duty).
- Faltan datos para analizar los dos lazos: setpoint y salida de humedad, temperatura ambiente, estado de la puerta y `Ts` medido.
- **Muy útil para la materia:** aceptar comandos por WebSocket para:
  - cambiar los setpoints, para probar respuesta a escalón;
  - cambiar Kp/Ti/Td en caliente, para comparar sintonías sin recompilar;
  - un **modo manual (lazo abierto)** que fije el duty, necesario para identificar la planta con un escalón. Al volver a automático la transferencia debe ser *bumpless*: iniciar `u[k−1]` con el duty manual.
- Agregar un botón para **descargar CSV** con tiempo, SP, PV, u y T_amb. Esos datos se pueden llevar a MATLAB para identificar el modelo y hacer las gráficas del reporte.

Para probar la HMI sin la placa: `node esp32-s3/sim/servidor_simulado.js` y abrir http://localhost:8080/.

## 🟡 15. WiFi y calidad de código

- En modo portal (`WIFI_AP_STA`), `manejarRaiz()` siempre redirige a la página de WiFi y el Serial imprime `IP: 0.0.0.0` ([`main.cpp:84`](../esp32-s3/src/main.cpp#L84)). La IP del portal es `WiFi.softAPIP()` (192.168.4.1).
- No hay forma de borrar las credenciales guardadas. Por ejemplo, mantener presionado un botón al arrancar podría hacer `preferences.clear()`.
- Falta `/logo.webp` en `data/` (404 en la página WiFi).
- Las variables globales están definidas en los headers. Funciona porque solo `main.cpp` los incluye, pero si otro `.cpp` los incluye habrá errores de *multiple definition*. Conviene declararlas con `extern` en `.h` y definirlas en `.cpp`.
- Variables sin uso: `usuarios_Permitidos`, `Tiempo_Maximo_Ejecucion`, `Tiempo_de_Subida_Sensor_*`, `WiFi_Conectado`.
- `src/main2.cpp` (lector de batería) está excluido de la compilación en `platformio.ini`. Si ya no se usa, conviene moverlo fuera de `src/`.
- Hay archivos sueltos que no son firmware: `src/xd.typ`, `src.zip`, `lib/main*.cpp`.

---

## Para el reporte de Control Digital

Cosas que el código ya permite, o permitiría con poco trabajo, y que suelen evaluarse en la materia:

1. **Identificación:** escalón de duty en lazo abierto (modo manual) → ajuste FOPDT con el método de dos puntos (28.3 % / 63.2 %) → K, τ, θ. Repetir con distintos escalones para mostrar la no linealidad.
2. **Discretización:** obtener `G(z)` con ZOH (`c2d(G, Ts, 'zoh')`) y justificar `Ts` con la regla Ts ≤ τ/10 o según el ancho de banda de lazo cerrado. Mostrar cómo el retardo θ queda como `z^{-d}`.
3. **Diseño del controlador:** derivar el PID en forma de velocidad desde `C(s)` con aproximación rectangular hacia atrás o con Tustin, y comparar las ecuaciones.
4. **Estabilidad en z:** polos de lazo cerrado dentro del círculo unitario, criterio de Jury, lugar de las raíces discreto y márgenes de ganancia y fase.
5. **Simulación vs. real:** simular el lazo discreto con saturación y anti-windup en MATLAB/Simulink y superponer los datos reales del CSV.
6. **Índices de desempeño:** sobreimpulso, tiempo de establecimiento, error en estado estacionario (meta ±1 °C, ±10 %HR) e IAE/ISE, comparando entre sintonías.
7. **Rechazo de perturbaciones:** abrir la puerta 30 s y comparar la recuperación con y sin feedforward.
