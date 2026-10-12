

## TECNOLÓGICO NACIONAL DE MÉXICO
Instituto tecnológico de Culiacán
Anteproyecto: Sistema Adaptativo de
Incubación para el Control Digital de
Temperatura y Humedad Relativa (SADI)
## Integrantes
## Fernández Angulo Jesús Iván
## Leon Ontiveros Leobardo Alexis
## Valdez Carrazco Gabriel Adrian
## Zavala Beltrán Jordi Jesús
## Carrera
## Ingeniería Mecatrónica
## Materia
## Control Digital
## Docente
## DR. OSWALDO CUEN
## TÉLLEZ
## Grupo
UA05 10:00 - 11:00 hrs
Vie 10:00 - 12:00 hrs
## Fecha
1 de octubre de 2026

## ÍNDICE
1.Introducción ........................................................................... 4
2.Planteamiento del Problema ........................................................... 5
3.Justificación Técnica y Comercial ..................................................... 6
4.Objetivos del proyecto ................................................................. 7
5.Marco Teórico Inicial .................................................................. 8
5.1Control Analógico ............................................................... 8
5.1.1Características Principales del Control Analógico .......................... 9
5.2Control digital .................................................................. 11
5.2.1Características Principales del Control Digital ............................ 12
5.3Transformada de Laplace ....................................................... 13
5.4Ecuaciones de diferencias ....................................................... 14
5.5Transformada Z ................................................................. 15
5.6Controlador PID ................................................................ 17
5.7Método de Sintonización: Ziegler-Nichols (Ganancia Límite o Lazo Cerrado) ... 18
5.7.1Causas de las Perturbaciones ............................................. 18
5.7.2¿Cómo Afectan las Perturbaciones al Sistema de Control? ................. 19
5.7.3Clasificación Máxima de las Perturbaciones según sus Fuentes ............ 19
5.8Microcontrolador ESP-32 ....................................................... 21
5.9Módulo sensor de temperatura y humedad DHT22 .............................. 23
5.10Modulación por Ancho de Pulso ................................................ 26
6.Diagrama de Bloques ................................................................. 28
7.Viabilidad Técnica ................................................................... 30
7.1Adquisición de materiales ....................................................... 30
7.2Riesgos principales ............................................................. 30
7.2.1Riesgos Internos .......................................................... 30
7.2.2Riesgos Externos ......................................................... 31
8.Presupuesto Estimado ................................................................ 32
9.Cronograma .......................................................................... 33
9.1Actividades del Proyecto ....................................................... 33
10.Alcance .............................................................................. 35
11.Referencias .......................................................................... 36
## 2

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## FIGURAS
Figura 1 DIAGRAMA DE BLOQUES DE UN CONTROL ANALÓGICO EN LAZO
CERRADO ...................................................................... 9
Figura 2DIAGRAMA DE BLOQUES LAZO DE CONTROL DIGITAL BASICO ....... 11
Tabla  1Fórmulas parámetros del controlador ............................................ 18
Figura 3ESP32-S3-DevKitC-1 with ESP32-S3-WROOM-1 Module ...................... 22
Figura 4ESP32-S3-DevKitC-1 Pin Layout ............................................... 22
Figura 5MODULO DHT22 CON SENSOR DE TEMPERATURA Y HUMEDAD ....... 23
Figura 6Pinout del sensor DHT22 ....................................................... 24
Tabla  2Funcionalidad de los pines del sensor DHT22 ................................... 24
Figura 7Representación señales PWM ................................................... 27
Figura 8Diagrama de bloques SADI ..................................................... 28
Tabla  3 Presupuesto inicial estimado de componentes del proyecto. ..................... 32
Figura 9Cronograma SADI .............................................................. 33
## 3

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 1.INTRODUCCIÓN
En  el  desarrollo  de  sistemas  de  control  y  su  implementación  en  entornos  industriales,  es
necesario tener presentes las propiedades intrínsecas de su funcionamiento. La naturaleza de los
sistemas en un entorno industrial tangible suele ser analógica: transfiere propiedades intensivas
o extensivas mediante el intercambio de energía, ya sea medida como energía cinética, potencial,
calorífica o de trabajo.
Sin embargo, los requerimientos y exigencias en la manipulación de dichos sistemas suelen
demandar un controlador computacional. Este debe obtener los datos del cambio de comporta-
miento de las variables de la planta, así como del comportamiento de la salida y la entrada, para
inferir la efectividad del proceso de control. Aquí surge una dificultad fundamental: la lógica
abstracta de los sistemas computacionales es exclusivamente discreta, pues se construye sobre
los principios de la lógica booleana. Registrar el comportamiento analógico de los sistemas
consume tiempo de procesamiento del controlador y exige precisión en el cálculo de las varia-
bles. Como resultado, puede generarse un sistema inestable o, en la mayoría de los casos, un
sistema nulamente aplicable a la teoría del control digital.
Para abordar lo anterior, se propone una interfaz de usuario diseñada mediante la conexión
al programa diseñado en un entorno HTML respectivamente enfocado a la parte virtual de la
supervisión del sistema; que permitirán controlar las variables de las funciones controladoras
PID y respectivamente para la parte física se implementara conexiones mediante pantalla LCD
y botones de inicio de proceso. La interacción entre el microcontrolador ESP32-S3 permitirá
comparar el comportamiento de un sistema de control con perturbaciones del entorno frente al
mismo sistema procesado numéricamente en un entorno controlado sin perturbaciones.
Debido  a  ello,  se  implementará  un  sistema  de  control  de  variables  mediante  servicios  de
conexión remota, usando bibliotecas que permiten la conexión a dispositivos cercanos por
protocolos WiFi.  Lo  anterior  conlleva  a  una  demostración  empírica  del  procesamiento  de
variables en tiempo real, mediante servicios de naturaleza open source (bibliotecas diseñadas
para el desarrollo de software aplicado directamente al código fuente del microcontrolador).
El desarrollo y conclusión de dichos procesos incentiva desglosar cada apartado para su uso
didáctico en un entorno estudiantil, y potenciar los procesos de desarrollo e implementación de
las herramientas existentes.
## 4

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 2.PLANTEAMIENTO DEL PROBLEMA
Los sistemas de incubación convencionales funcionan mediante cambios manuales, los cuales
solo permiten el control de encendido/apagado o niveles fijos de operación de los actuadores
(calefactor, ventilador o humidificador), lo cual genera limitantes como:
•Estrés térmico e inestabilidad del microclima: Los cambios abruptos de temperatura y
humedad al abrir la incubadora, las variaciones del ambiente o los ajustes tardíos provocan
fluctuaciones que afectan el desarrollo embrionario y reducen la tasa de eclosión.
•Mayor desgaste en el equipo: Mantener el sistema en un solo estado entregando toda la
potencia acorta su vida útil, además de incrementar el consumo eléctrico y el desgaste de los
componentes del sistema.
Por lo cual se implementará un sistema de incubación con control PID que regule automáti-
camente la temperatura, la humedad y la ventilación según lo requiera el entorno y la etapa
del proceso, manteniendo condiciones estables, reduciendo la intervención manual y evitando
variaciones bruscas que perjudiquen el desarrollo de los huevos.
## 5

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 3.JUSTIFICACIÓN TÉCNICA Y COMERCIAL
El problema central que aborda este proyecto radica en la falta de adaptabilidad continua en los
sistemas de incubación convencionales, los cuales operan bajo conmutaciones manuales o en
lazo abierto, provocando fluctuaciones de temperatura, humedad y ventilación que afectan el
desarrollo embrionario, reducen la tasa de eclosión y generan un consumo energético ineficiente.
Además, en el ámbito disciplinario, la ejecución de este trabajo permite consolidar competencias
avanzadas en el procesamiento digital de señales, el diseño de controladores PID en tiempo
discreto y la integración de hardware de potencia con sistemas embebidos, ofreciendo una
solución técnica versátil y transferible al desarrollo de tecnologías de incubación, climatización
y control térmico automatizado.
Desde el punto de vista comercial, la implementación de un sistema de incubación con control
PID permite mantener condiciones ambientales estables, reducir la intervención manual, dismi-
nuir pérdidas por variaciones bruscas y optimizar el consumo eléctrico, lo que se traduce en una
mayor productividad, menor desgaste de los actuadores y un producto más competitivo frente
a las incubadoras convencionales.
## 6

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 4.OBJETIVOS DEL PROYECTO
Objetivo general:
•Diseñar e implementar un sistema de control digital en lazo cerrado para regular la tempe-
ratura y la humedad relativa dentro de una incubadora, utilizando un microcontrolador ESP32
como controlador central, con el fin de mantener condiciones ambientales estables y rechazar
perturbaciones externas.
## Objetivos Específicos:
1.Modelar e identificar la dinámica de la planta y diseñar los controladores PID discretos para
los lazos de temperatura y humedad, considerando saturación de actuadores y tiempos de
muestreo adecuados.
2.Implementar el hardware y firmware del sistema, integrando sensores, actuadores (calefactor,
humidificador y ventilador), etapa de potencia aislada y el controlador en el ESP32.
3.Validar  experimentalmente  el  sistema  ante  cambios  de  setpoint  y  perturbaciones  como
apertura de puerta, verificando el cumplimiento de los márgenes de precisión y estabilidad
definidos en el alcance.
## 7

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 5.MARCO TEÓRICO INICIAL
## 5.1CONTROL ANALÓGICO
Un sistema de control automático es una interconexión de componentes físicos organizados de
tal forma que el arreglo resultante es capaz de regularse o controlarse por sí mismo, ajustando
el comportamiento de un proceso o planta para obtener una respuesta o salida específica ante
una entrada determinada. [1]
En la configuración más eficiente, conocida como sistema de lazo cerrado (o retroalimentado),
la acción de control depende directamente de la salida real del sistema.
Sus elementos y variables principales son:
1.Entrada de referencia (r(t) o Set-Point): Es el valor objetivo o comportamiento deseado
introducido al sistema por el usuario o por una etapa previa.
2.Sensor o Transductor: Dispositivo que detecta y mide continuamente la variable controlada
en la salida y la convierte en una señal de retroalimentación (b(t)), típicamente eléctrica o
neumática.
3.Comparador (Punto de Suma): Elemento que efectúa la resta algebraica entre la entrada de
referencia y la señal de retroalimentación (r(t) - b(t)), generando la señal de error (e(t)).
4.Controlador: Dispositivo encargado de procesar la señal de error para calcular y generar una
variable regulada (v(t)) o acción correctiva.
5.Elemento Final de Control (Actuador): Dispositivo (como una válvula de control o un
amplificador de potencia acoplado a un motor) que recibe la variable regulada y suministra
la cantidad de energía o masa necesaria a través de la variable manipulada (m(t)).
6.Proceso o Planta: El sistema físico real cuya variable controlada (y(t)) como temperatura,
nivel, presión, velocidad o posición se desea mantener en el valor deseado. [2]
## 8

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
Figura 1:  DIAGRAMA DE BLOQUES DE UN CONTROL ANALÓGICO EN LAZO
## CERRADO
El término analógico (o continuo en el tiempo) define la naturaleza matemática y física de las
señales que circulan por el sistema:
Continuidad temporal: Una señal analógica existe y está definida para todo instante de tiempo t
de forma ininterrumpida. A diferencia de los sistemas digitales o discretos (donde la información
solo se conoce en instantes específicos de muestreo kT mediante un muestreador y se codifica
de forma binaria), en lo analógico las variables varían suave y continuamente dentro de un rango
determinado.
Definición de Control Analógico: Se habla de un sistema de control analógico cuando todas
las etapas del lazo (medición, comparación, procesamiento del controlador y actuación) operan
íntegramente sobre señales continuas en el tiempo (como voltajes de 0 a 10 V, corrientes de 4
a 20 mA o presiones de 3 a 15 PSI. En este tipo de control no existen procesos de muestreo,
retenedores ni conversiones de análogo a digital (A/D) o de digital a análogo. [3]
## 5.1.1CARACTERÍSTICAS PRINCIPALES DEL CONTROL ANALÓGICO
Modelado  mediante  Ecuaciones  Diferenciales  y Transformada  de  Laplace: Al  trabajar  con
señales continuas, la dinámica del sistema analógico se representa matemáticamente en el
dominio del tiempo mediante ecuaciones diferenciales ordinarias. En el dominio de la frecuencia
compleja,  sus  bloques  y  controladores  se  caracterizan  por  funciones  de  transferencia  G(s)
basadas en la transformada de Laplace s.
Procesamiento Instantáneo en Tiempo Real: La respuesta del circuito controlador analógico
ante cualquier cambio en la señal de error ocurre de manera inmediata a través de la dinámica
electrónica o física de sus componentes, sin el retardo temporal asociado al período de muestreo
o al tiempo de cómputo de un procesador digital.
Dependencia Estructural del Hardware: La estrategia de control (las ganancias, las constantes
de integración y derivación) está fijada por los valores físicos de los componentes que integran
## 9

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
el circuito (resistencias, capacitores, inductores, amplificadores operacionales o componentes
neumáticos).
Rigidez y Limitaciones Físicas: Para modificar el comportamiento o la estructura de un contro-
lador analógico es necesario alterar físicamente los circuitos, cambiar componentes o ajustar
potenciómetros, a diferencia de un controlador digital en el que basta con modificar unas líneas
de código. Asimismo, está sujeto a derivas térmicas, envejecimiento o ruido propio de los
componentes electrónicos o neumáticos.
Tipos de Controladores Analógicos Clásicos: Incluyen las acciones de control básicas como
el Proporcional (P), Proporcional-Integral (PI), Proporcional-Derivativo (PD) y Proporcional-
Integral-Derivativo (PID), así como las redes de compensación de adelanto, atraso o atraso-
## -adelanto. [2]
## 10

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 5.2CONTROL DIGITAL
Un sistema de control automático mantiene el objetivo primario de regular de forma autónoma
una variable de salida (variable controlada) en un proceso o planta física para que siga fielmente
a una entrada de referencia (Set-Point) mediante la medición y corrección continua del error.
Sin embargo, el término digital (o discreto en el tiempo) introduce una transformación profunda
en  la  forma  en  que  se  representa,  transmite  y  procesa  la  información  dentro  del  lazo  de
regulación:
Discretización Temporal (Tiempo Discreto): A diferencia de las señales analógicas (que existen
de manera continua e ininterrumpida para todo instante t), en un sistema digital la información de
las variables solo se conoce, procesa y transmite en instantes de tiempo discretos determinados
por un periodo de muestreo T (es decir, en t = 0, T, 2T, ..., kT). Entre dos instantes de muestreo
consecutivos no existe transmisión activa de datos.
Cuantización en Amplitud: La señal física continúa capturada se convierte en un código de
información binario compuesto por una palabra digital de n bits. El rango dinámico de la señal
se divide en 2
## 푛
niveles discretos de amplitud, donde la resolución depende directamente del
número de bits del conversor.
El control digital (o control por computador) es un esquema de regulación en lazo cerrado
donde la etapa del controlador analógico tradicional (circuito de amplificadores operacionales o
componentes neumáticos) se reemplaza por un procesador digital (como un microcontrolador,
procesador digital de señales - DSP, autómata programable - PLC o computador industrial). [4]
En este esquema, el computador recibe la medida del proceso previa conversión digital, calcula
internamente la diferencia respecto al Set-Point, ejecuta un algoritmo de control expresado en
ecuaciones en diferencias y envía una señal de corrección codificada que se reconvierte en una
variable física continua para manipular la planta.
Figura 2: DIAGRAMA DE BLOQUES LAZO DE CONTROL DIGITAL BASICO
La Figura 2 muestra el diagrama en bloques de un sistema de control digital básico. Para realizar
el control, se mide la variable controlada (salida del proceso) mediante el sensor adecuado. La
salida del sensor, previamente convertida en señal eléctrica estándar se aplica al convertidor
de análogo a digital (A/D) a través de un circuito de muestreo y retención (S/H). El valor de
la señal discreta producida por el convertidor A/D se compara con el valor discreto prefijado
en el Set-Point (SP), el computador establece la diferencia (error) entre éstos valores y ejecuta
un programa en el cual se ha establecido el algoritmo de control deseado. De acuerdo con el
algoritmo de control, el computador proporciona una señal de salida discreta que es convertida
## 11

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
en una señal eléctrica continua mediante un convertidor de digital a análogo (D/A). Finalmente,
la salida del convertidor D/A, previamente acondicionada es aplicada al elemento final de
control (válvula de control), cuya acción sobre la variable manipulada, realiza la corrección
del error llevándolo a cero o a un valor mínimo aceptable. La estrategia de control se repite
constantemente y con una frecuencia determinada, realizando así el computador, el control del
proceso. [5]
## 5.2.1CARACTERÍSTICAS PRINCIPALES DEL CONTROL DIGITAL
Modelado mediante Ecuaciones en Diferencias y Transformada Z: En lugar de ecuaciones
diferenciales ordinarias y la transformada de Laplace s, los sistemas digitales se describen
matemáticamente mediante ecuaciones en diferencias finitas. Para su análisis en el dominio
de la frecuencia discreta se utiliza la transformada Z y las funciones de transferencia de pulso
## G(z). [4]
Dependencia Crítica del Periodo de Muestreo (T): La estabilidad, la respuesta transitoria y el
comportamiento global del sistema no dependen únicamente de la dinámica del proceso y de
las ganancias del controlador, sino también del valor asignado al periodo de muestreo T. Para
evitar la pérdida de información y el fenómeno de confusión de frecuencias (aliasing), el tiempo
T debe elegirse respetando el Teorema del Muestreo de Shannon-Nyquist.
Flexibilidad y Programabilidad por Software: Modificar la estrategia de control (por ejemplo,
pasar de una acción PID convencional a algoritmos avanzados como control Deadbeat, modelo
interno, asignación de polos o control en espacio de estados) o reajustar parámetros no requiere
cambiar el hardware físico ni alterar circuitos, sino simplemente actualizar unas líneas de código
en el programa.
Multitarea y Capacidad Multivariable: Un único procesador digital es capaz de controlar simul-
táneamente múltiples lazos de regulación independientes dentro de una planta, realizando al
mismo tiempo funciones complejas de supervisión, registro de datos e interfaz hombre-máquina
## (HMI).
Alta Inmunidad al Ruido y Estabilidad de Parámetros: Al procesar información en formato bi-
nario, los controladores digitales no sufren descalibraciones por deriva térmica, envejecimiento
de componentes electrónicos pasivos ni interferencias electromagnéticas moderadas en el algo-
ritmo.
Presencia de Retardos de Tiempo Intrínsecos: Las etapas de conversión analógico a digital y
el tiempo de cómputo del procesador introducen pequeñas demoras temporales que deben ser
consideradas al diseñar el sistema para evitar problemas de inestabilidad.
## 12

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 5.3TRANSFORMADA DE LAPLACE
La Transformada de Laplace es una transformación integral que convierte una función f(t)
definida en el dominio del tiempo continuo (para 푡≥0) en una función equivalente F(s) en
el dominio de la frecuencia compleja s. [3] En la teoría y la ingeniería de control, su objetivo
principal es transformar ecuaciones diferenciales lineales de orden n en ecuaciones algebraicas
sencillas. De esta manera, permite analizar la estabilidad, encontrar la respuesta temporal de los
sistemas físicos y obtener sus funciones de transferencia sin necesidad de resolver las ecuaciones
diferenciales en el dominio del tiempo directamente.
La transformada unilateral de Laplace se define operacionalmente mediante la siguiente integral
impropia:
## ℒ︀
## −1
## {푓(푡)}=퐹(푠)=∫
## ∞
## 0
## 푓(푡)푒
## −푠푡
## 푑푡
Donde los componentes significan lo siguiente:
ℒ︀: Representa el operador de la transformada de Laplace.
f(t): Es la función definida en el tiempo que se requiere transformar.
s: Es la variable compleja de frecuencia, definida como 푠=휎+푗휔, donde 휎 representa el
amortiguamiento o atenuación (eje real del plano s) y 휔es la frecuencia angular de oscilación
(eje imaginario).
## 푒
## −푠푡
: Es el núcleo de la transformación, encargado de ponderar exponencialmente la función
temporal para proyectarla al dominio s.
0: El límite inferior indica que la integración comienza justo antes de t = 0, lo que permite
evaluar funciones que presentan discontinuidades o impulsos (como la función delta de Dirac)
en el origen.
Para recuperar la función continua en el tiempo f(t) a partir de su representación F(s), se utiliza
la transformada inversa de Laplace, definida matemáticamente mediante la integral de contorno
en el plano complejo:
## ℒ︀
## −1
## {퐹(푠)}=푓(푡)=
## 1
## 2휋푗
## ∫
## 휎+푗∞
## 휎−푗∞
## 퐹(푠)푒
## 푠푡
## 푑푠
Transformada de Laplace(ℒ︀): Transforma ecuaciones diferenciales lineales ordinarias del do-
minio del tiempo continuo t en ecuaciones algebraicas en el dominio de la frecuencia compleja
s. Su aplicación central en control es la obtención de la función de transferencia G(s), la cual
relaciona la transformada de la salida con la de la entrada. A partir de G(s), es posible analizar
la estabilidad del sistema evaluando la ubicación de sus polos en el plano s y diseñar estructuras
de control analógico (como controladores PID y redes de compensación de fase).
## 13

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
Transformada Inversa de Laplace(ℒ︀
## −1
): Permite retornar del dominio s al dominio del tiempo t
para conocer la respuesta temporal continua y(t) (respuesta transitoria y de estado estable) ante
señales de prueba como escalón, rampa o impulso. [2]
## 5.4ECUACIONES DE DIFERENCIAS
En el estudio de los sistemas discretos y digitales, una ecuación en diferencias representa la
relación matemática entre la señal de salida actual de un sistema, sus valores anteriores (muestras
pasadas o retardadas) y la señal de entrada actual junto con sus valores pasados. [4]
Desempeñan en el dominio del tiempo discreto el mismo papel fundamental que las ecuaciones
diferenciales en el dominio continuo. Generalmente se formulan como ecuaciones recurrentes,
lo que permite calcular la respuesta del sistema en el instante actual de forma iterativa a partir
de la información almacenada de instantes previos.
La forma matemática estándar de una ecuación en diferencias lineal e invariante en el tiempo
de orden n expresada en términos de retardos es:
## 푦(푘)+푎
## 1
## 푦(푘−1)+푎
## 2
## 푦(푘−2)+...+푎
## 푛
## 푦(푘−푛)=푏
## 0
## 푥(푘)+푏
## 1
## 푥(푘−1)+...+푏
## 푚
## 푥(푘−푚)
En sumatoria explicita:
## 푦(푘)=∑
## 푚
## 푗=0
## 푏
## 푗
## 푥[푘−푗]−∑
## 푛
## 푖=0
## 푎
## 푖
## 푦[푘−푖]
En sumatoria implícita
## ∑
## 푛
## 푖=0
## 푎
## 푖
## 푦[푘−푖]=∑
## 푚
## 푗=0
## 푏
## 푗
## 푥[푘−푗]푐표푛푎
## 0
## =1
## Donde:
푘: Instante de muestreo actual (tiempo real t = kT, donde T es el período de muestreo).
푦(푘): Salida actual calculada por el controlador o respuesta del proceso en el instante k.
푦[푘−푖]:  Muestras  pasadas  o  retardadas  de  la  salida  en 푖  períodos  de  muestreo  (términos
autorregresivos).
푥[푘−푗]: Muestra actual (cuando j=0) y pasadas (cuando 푗>0) de la señal de entrada (términos
de media móvil).
## 푎
## 푖
: Coeficientes constantes asignados a las salidas pasadas (con 푎
## 0
## =1).
## 푏
## 푗
: Coeficientes constantes asignados a la entrada actual y sus retardos.
푛: Orden del sistema, determinado por el mayor retardo en la variable de salida.
푚: Mayor retardo en la variable de entrada (donde usualmente 푚≤푛 para que el sistema sea
físicamente realizable o causal). [5]
## 14

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 5.5TRANSFORMADA Z
La Transformada Z es una transformación matemática que convierte una secuencia de tiempo
discreto o una señal analógica muestreada x(kT) (o x(k)) en una representación equivalente X(z)
en el dominio de la frecuencia discreta o plano complejo z. [4]
En el ámbito de la ingeniería de control digital, la transformada Z desempeña un papel análogo
al de la transformada de Laplace en el estudio de sistemas de tiempo continuo. Su propósito
fundamental es transformar ecuaciones en diferencias finitas en ecuaciones algebraicas en el
dominio z, simplificando de manera notable el análisis de estabilidad, la respuesta transitoria y
la obtención de las funciones de transferencia de pulso G(z).
La transformada Z unilateral de una secuencia discreta de números x(k) o de una señal continúa
muestreada periódicamente x(t) en instantes t = kT (para 푘≥0)se define matemáticamente
mediante la siguiente serie infinita de potencias:
## 풵︀{푥(푘푇)}=푋(푧)=∑
## ∞
## 푘=0
## 푥(푘푇)푧
## −푘
o, expresada formalmente como una secuencia expandida de muestras:
## 풵︀{푥(푘푇)}=푋(푧)=∑
## ∞
## 푘=0
## 푥(푘푇)푧
## −푘
## =푥(0)+푥(1)푧
## −1
## +푥(2)푧
## −2
## +푥(3)푧
## −3
Donde los componentes representan:
풵︀: Es el operador de la transformada Z.
x(kT) o x(k): Es el valor numérico de la señal o secuencia evaluado en el k-ésimo instante de
muestreo, siendo T el período de muestreo.
z: Es la variable compleja discreta, vinculada con la variable de Laplace s mediante la ecuación
de mapeo 푧=푒
## 푠푇
## (donde 푠=휎+푗휔).
## 푧
## −푘
: Representa el operador de retardo temporal en el dominio discreto, correspondiente a un
desplazamiento de k períodos de muestreo.
La transformada inversa Z permite obtener la secuencia muestreada original x(kT) a partir de su
expresión en el dominio complejas X(z), ofreciendo la información de la señal exclusivamente
en los instantes concretos de muestreo.
La formulación matemática rigurosa se basa en la integral de inversión en el plano complejo:
## 풵︀
## −1
## {푋(푧)}=푥(푘푇)=
## 1
## 2휋푗
## ∮
## 퐶
## 푋(푧)푧
## 푘−1
## 푑푧
## Donde:
C: Es una trayectoria o contorno de integración cerrado en el plano complejo z que encierra
todos los polos de 푋(푧)푧
## 푘−1
dentro de su región de convergencia.
## 15

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
Transformada Z(풵︀): Constituye la herramienta matemática fundamental para tratar señales
muestreadas 푥(푘푇) y sistemas de tiempo discreto, relacionando los planos complejos mediante
la transformación 푧=푒
## 푠푇
. Se aplica para convertir ecuaciones en diferencias finitas en ecua-
ciones algebraicas manipulables y definir la función de transferencia de pulso G(z). A través de
G(z), se estudia la estabilidad del lazo digital (verificando que todos los polos de lazo cerrado
se encuentren dentro del círculo unitario |푧|<1 y se diseñan filtros y algoritmos de control
informático.
## Transformada Z Inversa (풵︀
## −1
): Permite convertir la salida expresada en el dominio z (Y(z))
de vuelta a la secuencia numérica en tiempo discreto y(kT). Su aplicación es indispensable
para resolver ecuaciones en diferencias recurrentes y determinar los valores exactos que toma
la variable de salida en cada instante de muestreo k, lo que permite trasladar los algoritmos
matemáticos a líneas de código ejecutables en microcontroladores o PLCs. [5]
## 16

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 5.6CONTROLADOR PID
Un controlador PID (Proporcional-Integral-Derivativo) es un algoritmo y dispositivo de control
por retroalimentación ampliamente utilizado en la industria para regular variables de proceso
(como temperatura, presión, flujo o velocidad). Su función es comparar continuamente el valor
medido en la salida con un valor deseado (Set-Point) para calcular la señal de error e(t) = r(t) -
y(t) y generar una acción correctiva que reduzca dicho error a cero. [3]
Estructura y Modos del Controlador PID
El controlador PID combina tres acciones de control distintas para calcular su salida v(t):
1.Acción Proporcional (P): Genera una salida directamente proporcional a la magnitud del
error actuante presente.
Función: Modifica la posición del elemento final de control de forma proporcional a la desvia-
ción. Acelera la respuesta del sistema, pero por sí sola no logra eliminar por completo el error
en estado estable.
2.Acción Integral (I): Genera una salida proporcional a la acumulación o integración del error
a lo largo del tiempo.
Función: Mueve el elemento final de control a una velocidad proporcional a la desviación. Su
objetivo principal es asegurar que el error de estado estable se reduzca a cero.
3.Acción Derivativa (D): Genera una salida proporcional a la velocidad de cambio o rapidez
con la que varía el error.
Función: Aporta una característica predictiva o anticipativa. Al responder a la tendencia del error
antes de que este crezca, incrementa el amortiguamiento, reduce el sobre impulso y mejora la
respuesta transitoria.
Ecuación del Controlador PID
En el dominio del tiempo continuo, la salida del controlador se expresa como:
## 푣(푡)=푘
## 푝
## 푒(푡)+
## 퐾
## 푝
## 푇
## 푖
## ∫
## 푡
## 0
## 푒(휏)푑휏+퐾
## 푝
## 푇
## 푑
## 푑푒(푡)
## 푑푡
En el dominio de la frecuencia compleja (Transformada de Laplace), su función de transferencia
## 퐺
## 푐
(푠) es:
## 퐺
## 푐
## (푠)=퐾
## 푝
## (1+
## 1
## 푇
## 푖
## 푠
## +푇
## 푑
## 푠=퐾
## 푝
## +
## 퐾
## 푖
## 푠
## +퐾
## 푑
## 푠
Donde los parámetros de ajuste son:
## 퐾
## 푝
: Ganancia proporcional.
## 푇
## 푖
: Tiempo integral(y 퐾푖=
## 퐾
## 푝
## 푇
## 푖
es la ganancia integral)
## 푇
## 푑
: Tiempo derivativo(y 퐾
## 푑
## =퐾
## 푑
## ∗푇
## 푑
es la ganancia derivativa) [2]
## 17

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 5.7MÉTODO DE SINTONIZACIÓN: ZIEGLER-NICHOLS (GA-
## NANCIA LÍMITE O LAZO CERRADO)
Sintonizar  un  controlador  consiste  en  determinar  los  valores  numéricos  adecuados  de  sus
parámetros (퐾
## 푝
## ,푇
## 푖
## ,푇
## 푑
) para que el lazo cerrado satisfaga las especificaciones de estabilidad y
respuesta deseadas. Uno de los métodos empíricos clásicos más utilizados es el método de la
ganancia límite de Ziegler-Nichols (sintonización en lazo cerrado). [1]
Procedimiento Paso a Paso:
1.Desactivar acciones integral y derivativa: En el controlador PID conectado en lazo cerrado
con la planta, se anula la acción integral haciéndola infinita (푇
## 푖
## =∞표퐾
## 푖
=0)y la derivativa
nula (푇
## 푑
=0)dejando únicamente activa la ganancia proporcional 퐾
## 푝
## .
2.Llevar al sistema a oscilación crítica: Se incrementa paulatinamente la ganancia proporcional
## 퐾
## 푝
hasta que la salida del sistema empiece a presentar una oscilación continua o Sostenida
(comportamiento en el límite de la estabilidad).
3.Medir parámetros límite:
•Se registra el valor de ganancia con el que ocurre dicha oscilación sostenida y se le denomina
ganancia última o máxima (퐾
## 푢
## ).
•Se mide el tiempo transcurrido en completar una oscilación completa y se le denomina período
último o máximo (푃
## 푢
## ).
4.Calcular parámetros del controlador: Utilizando los valores de 퐾
## 푢
y 푃
## 푢
obtenidos, se
aplican las fórmulas tabuladas por Ziegler y Nichols para ajustar el controlador deseado:
Tipo de controladorGanancia   propor-
cional(퐾
## 푝
## )
Tiempo integral(푇
## 푖
## )
Tiempo    derivati-
vo(푇
## 푑
## )
## Proporcional(p)
## 0.5퐾
## 푢
## --
## Proporcional-inte-
gral(PI)
## 0.45퐾
## 푢
## 푃
## 푢
## 1.2
## -
PID completo
## 0.6퐾
## 푢
## 푃
## 푢
## 2
## 푃
## 푢
## 8
Tabla  1: Fórmulas parámetros del controlador
Una perturbación (denotada comúnmente como p(t) o d(t)) se define en la teoría de control
como una señal externa indeseable e inevitable que actúa sobre el proceso o planta, afectando
negativamente el comportamiento de la variable controlada y(t) y provocando que esta se desvíe
del valor de referencia deseado o Set-Point. [2]
## 5.7.1CAUSAS DE LAS PERTURBACIONES
Las perturbaciones se originan por factores internos y externos a la planta bajo control, entre
los cuales destacan:
## 18

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
•Cambios  en  el  entorno  físico  o  ambiental: Variaciones  impredecibles  de  la  temperatura
ambiente, humedad, corrientes de aire o ráfagas de viento sobre estructuras (como antenas o
vehículos). Un ejemplo típico es la apertura de una puerta en un espacio climatizado, lo que
genera una entrada brusca de calor o frío.
•Variaciones en la carga o demanda del proceso: Cambios repentinos en el flujo de alimentación
## (푄
## 푖
), fluctuaciones en el par resistente de la carga mecánica acoplada a un motor de CD (푇
## 푝
## ),
o variaciones en el consumo de masa/energía del sistema.
•Inexactitudes e interferencias electromagnéticas: Ruido térmico o electromagnético super-
puesto  a  las  mediciones  registradas  por  los  sensores,  o  fluctuaciones  en  las  fuentes  de
alimentación eléctrica y neumática de los actuadores.
## 5.7.2¿CÓMO AFECTAN LAS PERTURBACIONES AL SISTEMA DE CONTROL?
El  impacto  de  una  perturbación  depende  fundamentalmente  de  la  arquitectura  del  lazo  de
regulación:
•Sistemas en Lazo Abierto: En estos sistemas la acción de control no depende de la salida real
del proceso. Por ello, los esquemas en lazo abierto son altamente sensibles e incapaces de
corregir el efecto de las perturbaciones o inexactitudes del modelo, dejando que la salida se
desvíe sin compensación alguna.
•Sistemas en Lazo Cerrado (Retroalimentados): La retroalimentación permite medir continua-
mente  la  salida  y(t)  y  compararla  con  la  referencia  r(t).  Si  actúa  una  perturbación,  el
comparador detecta el cambio en la señal de error e(t) y el controlador modifica la variable
manipulada m(t) para atenuar o erradicar la desviación.
•Efecto Particular en Sistemas Digitales: En un control por computador, entre un instante de
muestreo y el siguiente (kT y (k+1) T), el sistema opera en bucle abierto con una acción
de control constante fijada por el retenedor (ZOH). Si ocurre una perturbación entre dos
muestreos, su efecto no se puede compensar hasta que el convertidor A/D ejecute la siguiente
medición de la salida. [4]
## 5.7.3CLASIFICACIÓN MÁXIMA DE LAS PERTURBACIONES SEGÚN SUS
## FUENTES
A partir de la literatura técnica de control, las perturbaciones se pueden clasificar bajo tres
enfoques principales:
A. Según su Punto de Inserción u Origen en el Lazo
1.Perturbación en la Planta o de Carga (p(t) / d(t)): Ingresa directamente al proceso o entre el
actuador y la planta (por ejemplo, el torque de carga 푡
## 푝
(푠) aplicado a un motor de CD o la
apertura de válvulas de suministro).
2.Ruido de Medición (n(t)): Señal de alta frecuencia superpuesta a la salida real medida por
el sensor o transductor, lo cual introduce imprecisiones en la estimación del estado o en la
lectura del error.
## 19

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
3.Perturbaciones en la Alimentación / Actuador: Variaciones en el voltaje de suministro o en
la presión del aire comprimido del elemento final de control.
B. Según su Comportamiento Temporal / Forma de Onda (Pruebas Deterministas)
1.Escalón (Cambio Brusco de Carga): Representa una perturbación constante y repentina que
se mantiene en el tiempo (por ejemplo, la entrada de carga constante 푢
## 푝
(푡) o el cambio
abrupto de temperatura al abrir una puerta).
2.Rampa (Variación Continua): Representa una perturbación que varía a velocidad constante
a lo largo del tiempo.
3.Impulso (Golpe de Corta Duración): Fenómeno de gran magnitud y duración casi instantánea
(como un choque mecánico o un pico de voltaje transitorio).
C. Según su Contenido Frecuencial y Naturaleza Matemática
1.Perturbaciones de Baja Frecuencia (Derivas de Proceso): Cambios lentos y continuos en
el  punto  de  operación.  Son  rechazadas  eficazmente  incrementando  el  tipo  del  sistema
(añadiendo integradores en el controlador, como en las acciones PI o PID).
2.Perturbaciones/Ruido  de Alta  Frecuencia:  Interferencias  eléctricas  en  las  líneas  de  los
sensores. Para evitar que el proceso de muestreo las traduzca en ruido de baja frecuencia
(aliasing), se exige instalar filtros analógicos pasa-bajo (anti-aliasing) antes del muestreador.
3.Perturbaciones Estocásticas o Aleatorias: Señales aleatorias impredecibles que no pueden
moldearse mediante funciones deterministas simples. [3]
## 20

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 5.8MICROCONTROLADOR ESP-32
El ESP32 es un pequeño «cerebro» electrónico (un microcontrolador) creado por la empresa
Espressif Systems. Se ha vuelto muy popular porque incluye conexión a WiFi y Bluetooth
integrada directamente en el mismo chip, además de permitir la conexión de una gran variedad
de componentes como sensores, pantallas y motores. [6]
Dentro de esta familia de chips existe la serie ESP32-S3, que es una versión mejorada y más
potente. Cuenta con un procesador de doble núcleo más rápido, mayor capacidad para realizar
cálculos matemáticos complejos y más memoria para ejecutar programas pesados.
Para poder usar este chip fácilmente sin tener que soldar piezas complicadas, existe la placa de
desarrollo ESP32-S3-DevKitC-1-N16R8.
Características importantes del ESP32-S3-DevKitC-1-N16R8:
•Procesador dual-core Xtensa LX7 a 240 MHz
•Unidad de punto flotante (FPU) por hardware
•512 KB de SRAM interna
•8 MB de PSRAM (Octal SPI)
•16 MB de Flash
•Controlador PWM de alta resolución (LEDC) con hasta 8 canales y hasta 14 bits de resolución
•Varios timers de hardware de alta precisión
•ADC de 12 bits con múltiples canales
•Hasta 45 pines GPIO utilizables
•Voltaje de operación del chip: 3.0 V a 3.6 V (lógica a 3.3 V)
•Alimentación de la placa: 5 V por USB o 3.3 V por pines
•Dos puertos USB-C (uno UART y otro USB nativo OTG)
•WiFi 802.11 b/g/n y Bluetooth 5 LE integrados
•Soporte nativo de FreeRTOS para tareas en tiempo real
Este modelo se utiliza porque combina una potencia de cálculo elevada con una unidad de
punto flotante por hardware, lo que permite resolver de forma eficiente y precisa ecuaciones en
diferencias en tiempo discreto dentro de un lazo de control de iluminación. Su arquitectura de
doble núcleo facilita dedicar un núcleo exclusivamente a la ejecución periódica y determinista
del algoritmo de control, mientras el otro gestiona comunicaciones o tareas secundarias. La
presencia de 8 MB de PSRAM y 16 MB de Flash ofrece margen suficiente para algoritmos más
complejos, filtros de orden elevado o almacenamiento de datos históricos. Además, cuenta con
periféricos ideales para esta aplicación: un PWM de alta resolución para controlar la intensidad
de las luces con precisión, timers de hardware para generar el período de muestreo exacto. [6]
## 21

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
Figura 3: ESP32-S3-DevKitC-1 with ESP32-S3-WROOM-1 Module
Figura 4: ESP32-S3-DevKitC-1 Pin Layout
## 22

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 5.9MÓDULO SENSOR DE TEMPERATURA Y HUMEDAD DHT22
El módulo sensor de temperatura y humedad DHT22 (también conocido como AM2302) es un
módulo compacto diseñado para medir la temperatura y la humedad relativa del ambiente. Es
un componente estándar ampliamente utilizado en proyectos de electrónica y kits de desarrollo
con microcontroladores como Arduino, ESP32 o Raspberry Pi. [7]
Figura 5: MODULO DHT22 CON SENSOR DE TEMPERATURA Y HUMEDAD
El funcionamiento del sensor se basa en la combinación de un sensor capacitivo de humedad y
un termistor NTC:
•Sensor de Humedad Capacitivo: El elemento sensible a la humedad es un condensador cuyo
material dieléctrico absorbe o libera vapor de agua según la humedad relativa del ambiente.
Esto provoca una variación en la capacitancia, que es medida y convertida internamente en
una señal digital.
•Termistor NTC: Para la temperatura, el DHT22 incorpora un termistor de coeficiente negativo
(NTC), cuya resistencia eléctrica disminuye a medida que aumenta la temperatura. Esta
variación es acondicionada y digitalizada internamente.
•Conversión Analógica-Digital y Calibración: El módulo incluye un microcontrolador interno
que realiza la conversión analógica-digital, aplica coeficientes de calibración almacenados en
memoria OTP y entrega los datos de temperatura y humedad ya procesados.
•Comunicación Digital: A diferencia de los sensores analógicos, el DHT22 transmite los datos
mediante un protocolo digital de un solo hilo (single-wire). El microcontrolador externo envía
una señal de inicio y luego recibe una trama de 40 bits con la información de humedad y
temperatura.
## 23

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
Figura 6: Pinout del sensor DHT22
El módulo dispone de una tira de 3 pines macho identificados en la serigrafía de la placa:
PINIdentificaciónFunciónConexión típica
SData/señalSalida digital de datosPin  digital  del  microcontro-
lador
CentralVCCAlimentación positiva3.3 V o 5 V de la placa de
desarrollo
-GroundTierra/MasaPin Gnd de la fuente o del
microcontrolador
Tabla  2: Funcionalidad de los pines del sensor DHT22
## 24

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## Características Técnicas Principales
•Elemento Sensor: Sensor capacitivo de humedad y termistor NTC (modelo AM2302).
•Resistencia de Carga Incorporada: Resistencia pull-up de 10 kΩ en el PCB (en algunos
módulos; si no, se requiere añadir una resistencia externa de 4.7 kΩ a 10 kΩ entre DATA
y VCC).
•Tensión de Alimentación (VCC): De 3.3 V a 5 V CC.
•Tipo de Salida: Señal digital de un solo hilo (single-wire).
•Rango de Medición de Humedad: 0 % a 100 % HR.
•Precisión de Humedad: ±2 % HR (típica), ±5 % HR (máxima).
•Rango de Medición de Temperatura: −40 °C a 80 °C.
•Precisión de Temperatura: ±0.5 °C.
•Resolución: 0.1 % HR para humedad y 0.1 °C para temperatura.
•Tiempo de Muestreo: 2 segundos (0.5 Hz) mínimo entre lecturas.
•Tiempo de Respuesta: 5 a 10 segundos para humedad; 2 a 5 segundos para temperatura
## (aproximado).
•Pico de Respuesta Espectral: No aplica (no es un sensor óptico). [7]
## 25

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 5.10MODULACIÓN POR ANCHO DE PULSO
La Modulación por Ancho de Pulso (o PWM, por sus siglas en inglés Pulse-Width Modulation)
es una técnica digital utilizada para controlar la cantidad de potencia promedio entregada a una
carga eléctrica o para simular señales de tipo analógico utilizando salidas puramente digitales.
## [8]
En lugar de variar directamente el nivel de voltaje continuo de forma analógica, una señal PWM
conmuta rápidamente la alimentación entre un estado totalmente activo (alto / ON) y un estado
totalmente inactivo (bajo / OFF) manteniendo una frecuencia fija.
El ciclo de trabajo (duty cycle) de una señal periódica es el ancho de su parte positiva, en relación
con el período. Está expresado en porcentaje, por tanto, un duty cycle de 10% indica que está
10 de 100 a nivel alto.
Para comprender el funcionamiento del PWM, el parámetro fundamental es el ciclo de trabajo
(duty cycle), el cual representa la proporción de tiempo en la que la señal se mantiene en estado
alto (푇
## 표푛
) respecto al período total del ciclo
Fórmula del ciclo de trabajo:
## 퐷푢푡푦퐶푦푐푙푒(%)=
## 푡
## 표푛
## 푇
## ×100
Voltaje Promedio Equvalente: El voltaje promedio suministrado al componente se calcula en
función del ciclo de trabajo:
## 푉
## 푝푟표푚푒푑푖표
## =(푉
max
## −푉
min
## )×(
## 퐷푢푡푦퐶푦푐푙푒
## 100
## )
Básicamente, consiste en activar una salida digital durante un tiempo y mantenerla apagada
durante el resto, generando así pulsos positivos que se repiten de manera constante. Por tanto, la
frecuencia es constante (es decir, el tiempo entre disparo de pulsos), mientras que se hace variar
la anchura del pulso, el duty cycle. El promedio de esta tensión de salida, a lo largo del tiempo,
será igual al valor analógico deseado.
## 26

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
Figura 7: Representación señales PWM
Existen dos métodos principales para generar ondas PWM:
•Hardware / Circuito Analógico: Se utiliza un comparador que contrasta una tensión continua
de referencia (que define el nivel deseado) contra una señal oscilatoria de forma triangular
o diente de sierra. Mientras la señal de referencia supere a la onda oscilatoria, la salida del
comparador permanece en alto.
•Microcontroladores y Placas de Desarrollo: Los sistemas digitales como Arduino utilizan
temporizadores internos en pines digitales específicos (identificados comúnmente con una
virgulilla ~, tales como los pines 3, 5, 6, 9, 10 y 11 en un Arduino UNO) para generar ondas
cuadradas de ciclo de trabajo variable. [9]
A diferencia del control analógico tradicional donde dispositivos como los reóstatos o transis-
tores operados en zona lineal regulan la energía disipando el exceso en forma de calor, el PWM
es sumamente eficiente en términos de energía:
Cuando el elemento conmutador (transistor) está completamente en Corte (OFF), la corriente
es cero (I=0), por lo que la potencia disipada es nula (푃=푉•퐼=0).
Cuando está en Saturación (ON), la caída de voltaje en el transistor es casi nula (푉≈0),
disipando muy poca energía en forma de calor.
En esta aplicación de control de temperatura y humedad, el PWM se utiliza para regular de
forma precisa y eficiente la intensidad calorifica de la lampara. El sistema mide continuamente la
temperatura mediante el sensor DTH-22 y, a partir de esa información, determina si es necesario
aumentar o reducir la disipación de calor de la lampara.
Para accionar la lámpara de calefacción se puede emplear un MOSFET de potencia IRF640
[10], aislado mediante un optoacoplador PC817 [11].
## 27

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 6.DIAGRAMA DE BLOQUES
Figura 8: Diagrama de bloques SADI
El diagrama representa el sistema de control diseñado para mantener estables la temperatura y la
humedad dentro de la incubadora. El sistema funciona con dos circuitos de control que trabajan
en paralelo, pero que interactúan entre sí en la misma cámara.
1.Entradas y Comparación: En el lado izquierdo se establecen los valores deseados: “Tem-
peratura deseada” y “Humedad deseada”. Estos valores ingresan a unos comparadores (los
círculos con “+” y “-”), donde se restan de las mediciones reales que envían los sensores.
La diferencia entre lo que queremos y lo que tenemos es el “error”, que es lo que el sistema
debe corregir.
2.Control Digital (Recuadro punteado): El error de cada variable entra a un bloque PID,
que calcula qué tan fuerte debe actuar el sistema. Esta orden pasa al “Controlador (ESP32)”,
que es el cerebro del proyecto y se encarga de ejecutar el algoritmo de control y generar las
señales necesarias.
3.Etapa de Potencia: Como el ESP32 trabaja con señales de bajo voltaje, necesita un bloque
de “Potencia (Optoacoplador con MOSFET)”. Esta etapa funciona como un interruptor
electrónico seguro: aísla el microcontrolador de la parte de alta potencia y permite controlar
la energía que llega a los actuadores.
4.Actuadores: Aquí se encuentran los dos dispositivos que modifican el ambiente:
•Calefactor (representado con un foco): Se encarga de subir la temperatura.
•Humidificador (ultrasónico de membrana): Se encarga de subir la humedad.
5.La Planta (Incubadora): Es la cámara física donde están los huevos. Este bloque es muy
importante porque muestra que ambos actuadores afectan al mismo ambiente. Las flechas
internas indican cómo interactúan:
•El calefactor sube la temperatura, pero baja la humedad.
•El humidificador sube la humedad, pero baja un poco la temperatura.
6.Sensores  (Retroalimentación):  Finalmente,  los  sensores  de  temperatura  y  humedad
(DHT22) miden las condiciones reales dentro de la incubadora. Estas mediciones regresan a
## 28

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
los comparadores del inicio, cerrando así el ciclo de control. Si la temperatura baja, el sistema
lo detecta y enciende el calefactor; si la humedad baja, enciende el humidificador.
En resumen: El diagrama muestra un sistema de control en lazo cerrado que no solo enciende
y apaga dispositivos, sino que mide constantemente el resultado de sus acciones y se corrige
a sí mismo, teniendo en cuenta que calentar o humidificar afecta a ambas variables al mismo
tiempo.
## 29

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 7.VIABILIDAD TÉCNICA
## 7.1ADQUISICIÓN DE MATERIALES
La  elaboración  del  sistema  de  control  de  temperatura  y  humedad  para  incubadora  posee
como arquitectura principal una serie de componentes electrónicos para el control de señales
y un sistema de potencia para el control de magnitudes de mayor voltaje y corriente. Entre
los componentes que integran el sistema se encuentran: un microcontrolador con capacidad
de procesamiento y módulos de entrada/salida; sensores de temperatura y humedad relativa;
un módulo de acondicionamiento y filtrado para las señales analógicas; actuadores como un
elemento calefactor, un humidificador ultrasónico y un ventilador de circulación; drivers de
potencia basados en transistores o relés para gobernar los actuadores; una fuente de alimentación
con regulación para los distintos niveles de voltaje; y una interfaz de usuario compuesta por
una pantalla y pulsadores para el ajuste de los valores de referencia. Cada uno de dichos
componentes puede adquirirse de forma comercial debido a que se trata de elementos de uso
común en proyectos de electrónica y automatización, disponibles tanto en tiendas especializadas
como en plataformas de venta en línea. La facilidad para conseguirlos radica en que no requieren
fabricación especial ni pedidos personalizados, y suelen formar parte del inventario habitual de
los proveedores. Asimismo, su costo es relativamente bajo, ya que son componentes producidos
en masa y ampliamente utilizados en aplicaciones similares, lo que los hace accesibles para
proyectos de diversa índole. En conjunto, la obtención de los materiales para este sistema resulta
sencilla y económica, sin necesidad de recurrir a componentes exóticos o de difícil consecución.
## 7.2RIESGOS PRINCIPALES
Debido a que un sistema de control discreto debe de poseer una base física y computacional.
Existen riesgos pertinentes que se deben de considerar para la manipulación del sistema de
control, lo que es considerado como la manipulación interna del sistema o fallos críticos en la
interacción externa del ambiente del sistema de control.
## 7.2.1RIESGOS INTERNOS
Los riesgos internos se clasifican como fallos en la estructura del sistema donde el usuario final
no posee interacción alguna con la arquitectura de procesamiento del sistema.
1.Daños por degradación de los componentes electrónicos: Existen factores principales a
considerar en las condiciones ambientales presentes en el uso del sistema. Debido a que las
diferentes propiedades físicas presentes en el ambiente pueden alterar los niveles de lectura
o comportamientos de los sistemas preactuadores del sistema.
Por  ejemplo:  niveles  altos  de  humedad  o  temperatura  ambiental  pueden  interferir  con
las capacidades de procesamiento del sistema. Los componentes electrónicos que principal-
mente se ven afectados por las propiedades físicas anteriormente mencionadas son: sistemas
microcontroladores, sensores y componentes que no poseen un encapsulado robusto de los
materiales internos.
## 30

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
Se tiene presente la posibilidad de que al momento de que una variable sistema se vea
alterada por los cambios en el procesamiento o ejecución de dicho componentes electrónicos
consecuentes por alteraciones ambientales, la posibilidad de alteración en las variables de
perturbación y sensoriales pueden alterar al sistema provocando errores en la salida del
sistema donde bajo su modelo matemático de origen se predice como estado estable imposi-
bilitando llegar al punto de consigna requerido por la entrada solicitada al sistema. Cabe
mencionar que si la entrada del sistema es una entrada de voltaje, este ultimo puede provocar
exigencias mayores o menores al preactuador o actuador pertinente lo cual puede afectar la
calidad de vida de los materiales presentes en dichos materiales.
2.Desconexión de componentes electrónicos: El principal uso del sistema de control de
temperatura y humedad propuesta en el desarrollo del sistema de incubación. Al depender de
una arquitectura física transportable, las vibraciones mecánicas externas al sistema provocan
alteraciones en la unión física de los componentes electrónicos especialmente en sus zonas de
soldadura. Provocando dentro de los procesos de procesamiento del microcontrolador; falsas
lecturas de los componentes dependientes de las propiedades u variables asociadas dentro
de modelo matemático del sistema de control. Lo que permite al cálculo de las ecuaciones
de diferencias del sistema llegar a un punto de margen donde el sistema empiece a generar
desestabilización al comportamiento del sistema a controlar.
## 7.2.2RIESGOS EXTERNOS
Los riesgos externos se clasifican como fallos en la estructura del sistema donde el usuario final
mantiene interacción con la arquitectura.
1.Alimentación de voltaje excesiva: La variable principal que otorga a los sistemas de control
eléctricos/electrónicos es la consideración del nivel de entrada de voltaje. Debido a que cada
componentes incorporados dentro de un sistema de control ya sea analógico u digital poseen
un limite de alimentación de voltaje o corriente para la ejecución de las operaciones desti-
nadas a la integración dentro del sistema. Dentro del desglose del funcionamiento del sistema
se otorgaran parámetros del nivel de entrada de voltaje requerido para el funcionamiento del
sistema se visualizara la incorporación de una de corriente continua de un rango máximo
de 12V. Se le proporcionará al usuario un señalización del margen para la alimentación de
voltaje al sistema.
2.Lámparas de alto nivel de consumo:  Una  consideración  principal  dentro  del  sistema
incubadora, es el desgaste de los materiales que componen al calentador principal que
proporcionará el cambio de temperatura dentro del los parámetros dimensionales del sistema.
Al incorporarse un lampara incandescente como componente principal que tendrá la función
de calentador; se debe proporcionar información al usuario de las capacidades especificas
requeridas (Nivel de voltaje requerido, consumo de watts del la lámpara, etc). La señalización
efectiva de los parámetros del funcionamiento del calentador sirve para la sustitución de
dicho actuador cuando el periodo de vida útil de dicha lámpara sea concluido. Y se ejecuta
de nueva cuenta los procesos de las variables de control digital con la normalidad esperable.
Ya que al hacer caso omiso en la sustitución del componente calentador con los parámetros
especificados; puede dañar la calidad de la eclosión del embrión.
## 31

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 8.PRESUPUESTO ESTIMADO
Presupuesto del Proyecto — Incubadora
CategoríaComponenteCant.P. Unit.ImporteProveedor
PotenciaFuente de Alimentación Conmutada 12V 10A
## 120W
1$162.00$162.00Mercado Libre
PotenciaVentilador DC 12V 80mm1$68.00$68.00Mercado Libre
PotenciaFoco Halógeno H4 12V 60/55W Hella1$65.00$65.00Mercado Libre
PotenciaPlaca Control Humidificador Ultrasónico1$60.00$60.00Mercado Libre
PotenciaMódulo Relevador (Relay) 5V 1 Canal 10A1$64.86$64.86Mercado Libre
PotenciaMódulos Puente H BTS7960 43A2$95.36$190.72Mercado Libre
ElectrónicaPantalla LCD 16x2 con I2C1$103.00$103.00Mercado Libre
ElectrónicaSensor DHT22 AM2302 (Temp. y Humedad)2$58.05$116.1Mercado Libre
ElectrónicaConector Soquet Cerámico H4 3 Pines1$45.00$45.00Mercado Libre
ElectrónicaESP32-S3-DevKitC-1-N8R81$382.11$382.11Amazon
## TOTAL:$1,256.79
Tabla  3:  Presupuesto inicial estimado de componentes del proyecto.
La Tabla   3  detalla  el  presupuesto  estimado  para  la  construcción  de  la  incubadora.  Los
componentes están divididos en dos categorías: Potencia (fuente de alimentación, ventilador,
foco halógeno, humidificador y módulos de control) y Electrónica (microcontrolador ESP32,
sensores DHT22, pantalla LCD y conectores). El costo total asciende a $1,256.79, y todos los
elementos se encuentran actualmente cotizados. Esta selección de materiales permite cumplir
con los requisitos de precisión y control establecidos en el proyecto, manteniendo un costo
accesible.
## 32

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 9.CRONOGRAMA
Gestión de Materiales
Cotización y pedidos online
Recepción y revisión de envíos
Mecánica (Estructura)
Construcción de cámara aislada
Montaje de actuadores y sensores
Electrónica (Sensores)
Cableado y lecturas DHT22
Potencia (Actuadores)
Diseño y circuito principal
Integración de relés y PWM
Programación de Control
Modelado teórico y discretización Z
Pruebas en lazo abierto (Escalón)
Implementación PID (Temp/Hum)
Integración y Pruebas
Ensamblaje y sintonización fina
Pruebas de perturbación cruzada
Análisis y reporte final
## 28051219260209162330
SepOctNov
## Primer Avance
(Modelo)
## Entrega Final
29 nov
Figura 9: Cronograma SADI
El cronograma de actividades (Figura 9) establece la planificación temporal del proyecto a
lo largo de 9 semanas, estructurado bajo una filosofía de ingeniería concurrente. Dado que el
equipo consta de 4 integrantes con carga compartida en otros proyectos, el flujo de trabajo
paraleliza el diseño mecánico, la electrónica y la base matemática durante el primer mes. Esta
distribución estratégica evita tiempos muertos y permite que el prototipo físico esté listo justo
a tiempo para converger en la fase crítica de pruebas de control y sintonización.
## 9.1ACTIVIDADES DEL PROYECTO
Las actividades principales se distribuyen estratégicamente para maximizar el trabajo en para-
lelo:
## 33

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
1.Gestión de Materiales: Fase logística para la adquisición del hardware necesario.
1.Cotización y pedidos online: Búsqueda de proveedores y compra de los componentes
electrónicos y electromecánicos.
2.Recepción y revisión de envíos: Tiempos de espera de paqueterías y comprobación
funcional de las piezas al llegar.
2.Mecánica (Estructura): Construcción de la cámara física para albergar el sistema.
1.Construcción de cámara aislada: Fabricación del chasis o cajón a escala con el aisla-
miento térmico adecuado.
2.Montaje de actuadores y sensores: Instalación y distribución del calefactor, humidifi-
cador y sensores dentro de la estructura.
3.Electrónica (Sensores): Acondicionamiento y lectura de las variables del microclima.
1.Cableado y lecturas DHT22: Conexión física y configuración básica del microcontro-
lador ESP32 para adquirir los datos de temperatura y humedad.
4.Potencia (Actuadores): Desarrollo de la etapa de potencia para alimentar y accionar el
sistema.
1.Circuito principal y distribución: Armado de la red de suministro eléctrico para los
actuadores y la lógica de control.
2.Integración de relés y PWM: Implementación de los circuitos de conmutación necesa-
rios para regular la intensidad del calefactor y activar el humidificador.
5.Programación de Control: Desarrollo de los algoritmos para regular la temperatura y
humedad.
1.Modelado  teórico  y  discretización  Z:  Obtención  del  modelo  matemático  y  de  las
funciones de transferencia del sistema en tiempo discreto.
2.Pruebas en lazo abierto (Escalón): Experimentación básica para conocer el comporta-
miento físico de la planta.
3.Implementación PID (Temp/Hum): Programación de los controladores digitales en el
ESP32 para cerrar los lazos de control.
6.Integración y Pruebas: Fase de convergencia para validar el funcionamiento del prototipo.
1.Ensamblaje y sintonización fina: Unión del hardware y software, y ajuste práctico de
los parámetros del controlador PID.
2.Pruebas de perturbación cruzada: Aplicación de alteraciones controladas (como abrir
la puerta) para comprobar la estabilidad del sistema.
3.Análisis y reporte final: Recopilación de los resultados de desempeño y redacción del
documento final.
## 34

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 10.ALCANCE
El sistema de control de temperatura y humedad para incubadora tiene como alcance principal
el monitoreo, regulación y despliegue de las variables climáticas dentro de la cámara de incu-
bación, con el fin de mantener condiciones estables y adecuadas para el desarrollo de los huevos
o el cultivo biológico. Sin embargo, su implementación no abarca los siguientes aspectos:
•Diseño mecánico de la incubadora: No incluye las bandejas, el sistema de volteo de huevos
ni la distribución interna del aire.
•Sistema de respaldo energético: no contempla fuentes de alimentación ininterrumpida (UPS)
ni generación autónoma ante cortes de energía.
•Control de otros gases: no regula niveles de CO₂, O₂ ni otros parámetros atmosféricos
distintos de temperatura y humedad.
•Sistemas de seguridad intrínseca: no incorpora protecciones contra fallos catastróficos (cor-
tocircuitos, sobrecalentamiento extremo) más allá de las protecciones básicas del hardware.
En cuanto al nivel de precisión esperado, se establecen márgenes alcanzables con componentes
de bajo costo y un algoritmo de control bien sintonizado, sin aspirar a exactitudes de grado
industrial:
•Temperatura: se espera una precisión de ±1 °C en el rango de operación (aproximadamente
35 a 40 °C), con una estabilidad en régimen permanente de ±0.5 °C.
•Humedad relativa: se espera una precisión de ±10 % HR en el rango de 40 a 70 % HR, con
una estabilidad de ±5 % HR una vez alcanzado el punto de consigna.
•Tiempo de respuesta: el sistema debe corregir perturbaciones como la apertura de la puerta
en un tiempo razonable, procurando recuperar los valores de referencia en pocos minutos.
•Resolución de medida: la resolución mínima de los sensores debe ser de 0.1 °C y 0.1 % HR
para permitir un control adecuado, aunque la exactitud absoluta sea menor.
Estos márgenes son alcanzables con sensores de bajo costo y un algoritmo de control PID bien
sintonizado, siempre que se garantice una adecuada circulación de aire y un aislamiento térmico
razonable en la incubadora. El objetivo no es competir con equipos comerciales, sino demostrar
el dominio del diseño de sistemas de control en un contexto académico.
## 35

Anteproyecto: Sistema Adaptativo de Incubación para el Control Digital de Temperatura y Humedad Relativa
## (SADI)
Instituto tecnológico de Culiacán
## 11.REFERENCIAS
[1]R. Hernández Gaviño, Introducción a los sistemas de control: Conceptos, aplicaciones y
simulación con MATLAB, 1st ed. México: Pearson Educación, 2010.
[2]N. S. Nise, Control Systems Engineering, 6th ed. Hoboken, NJ: John Wiley & Sons, Inc.,
## 2011.
[3]K. Ogata, Modern Control Engineering, 5th ed. Upper Saddle River, NJ: Prentice Hall,
## 2010.
[4]K. Ogata, Sistemas de Control en Tiempo Discreto, 2.ª ed. Naucalpan de Juárez, Edo. de
México, México: Prentice Hall Hispanoamericana, S.A., 1996.
[5]L. E. García Jaimes, Control Digital: Teoría y Práctica, 2nd ed. Medellín, Colombia:
## Politécnico Colombiano Jaime Isaza Cadavid, 2009.
[6]Espressif  Systems,  «esp-dev-kits  Documentation:  ESP32-S3  Development  Kits  User
## Guide». Shanghai, China, 2022.
[7]Aosong Electronics Co., Ltd., «Digital-output relative humidity & temperature sensor/
module DHT22 (DHT22 also named as AM2302)». 2026. [En línea].  Disponible en:
https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf
[8]SparkFun Electronics, «Pulse Width Modulation». [En línea]. Disponible en: https://learn.
sparkfun.com/tutorials/pulse-width-modulation
[9]Prometec, «PWM - Para Arduino». [En línea]. Disponible en: https://www.prometec.net/
pwm/
## [10]
STMicroelectronics, «IRF640, IRF640FP N-channel 200V - 0.150 Ω - 18A TO-220/
TO-220FP Mesh overlay Power MOSFET». septiembre de 2006.
[11]SHARP Corporation, «PC817XxNSZ1B Series Photocoupler». enero de 2018.
## 36