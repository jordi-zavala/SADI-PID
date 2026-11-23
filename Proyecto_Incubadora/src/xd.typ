#set document(title: "Reporte de Propiedades Físicas")
#set page(paper: "a4", margin: 2.5cm)
#set text(lang: "es", size: 11pt)
#set par(justify: true)
#set heading(numbering: none)
#show link: set text(fill: blue.darken(30%))

#align(center)[
  #text(18pt, weight: "bold")[Reporte de Propiedades Físicas] \
  #text(12pt)[Cálculo analítico-computacional a partir de archivos STEP] \
  #v(0.6em)
  #text(10pt)[Generado automáticamente con `CadQuery` + `OpenCASCADE (OCCT)`]
]

#align(right)[_19/11/2026_]

= Resumen

Este documento reporta el cálculo de las propiedades físicas fundamentales ---volumen, masa, centro de gravedad y tensor de inercia--- de uno o más sólidos definidos en archivos STEP. Las integrales volumétricas se evalúan de forma exacta sobre la representación B-Rep del sólido mediante el kernel OpenCASCADE (OCCT).

Densidad uniforme asumida: $rho = 7850.0 "kg/m"^3 = 7.8500 times 10^(-6) "kg/mm"^3$. Se procesaron *1* archivo(s).

#outline(title: "Contenido", indent: auto)
#pagebreak()

== Metodología y formulación
#pagebreak()
== Volumen
$ V = integral.triple_Omega dif V $

== Masa
Con densidad uniforme $rho$: $ m = rho V $

== Centro de gravedad.

$ bar(x) = 1/V integral.triple_Omega x dif V, quad bar(y) = 1/V integral.triple_Omega y dif V, quad bar(z) = 1/V integral.triple_Omega z dif V $

== Tensor de inercia
Respecto al origen $O$:
$ [I]_O = integral.triple_Omega rho mat(y^2 + z^2, -x y, -x z;     -x y, x^2 + z^2, -y z;     -x z, -y z, x^2 + y^2) dif V $

Respecto al centro de gravedad $G$ (teorema de Steiner / ejes paralelos):
$ I_(x x)^(G) = I_(x x)^(O) - m(bar(y)^2 + bar(z)^2), quad I_(y y)^(G) = I_(y y)^(O) - m(bar(x)^2 + bar(z)^2), quad I_(z z)^(G) = I_(z z)^(O) - m(bar(x)^2 + bar(y)^2) $
$ I_(x y)^(G) = I_(x y)^(O) + m bar(x) bar(y), quad I_(x z)^(G) = I_(x z)^(O) + m bar(x) bar(z), quad I_(y z)^(G) = I_(y z)^(O) + m bar(y) bar(z) $

== Resultados

== Archivo 1: `Linkage_Bolt.step`

=== Volumen
$ V = 41689.522 "mm"^3 = 4.1690 times 10^(-5) "m"^3 $

=== Masa
$ m = rho V = 7850.0 "kg/m"^3 dot 4.1690 times 10^(-5) "m"^3 = 3.2726 times 10^(-1) "kg" $

=== Centro de gravedad
$ bar(x) = -0.0000 "mm", quad bar(y) = -0.0000 "mm", quad bar(z) = -65.0000 "mm" $

=== Tensor de inercia respecto a $G$
$ [I]_G = mat(-3.3253 times 10^(2), 1.8233 times 10^(-7), 3.9387 times 10^(-3); 1.8233 times 10^(-7), -3.3253 times 10^(2), 6.9691 times 10^(-5); 3.9387 times 10^(-3), 6.9691 times 10^(-5), 1.7612 times 10^(1)) "kg" dot "mm"^2 $

Momentos principales (autovalores):
$ I_1 = -3.3253 times 10^(2), quad I_2 = -3.3253 times 10^(2), quad I_3 = 1.7612 times 10^(1) quad "kg" dot "mm"^2 $

=== Tabla resumen
#table(
  columns: 2,
  align: (left, right),
  inset: 4pt,
  table.hline(stroke: 0.5pt),
  [*Propiedad*], [*Valor*],
  table.hline(stroke: 0.5pt),
  [Volumen], [$V = 41689.522 "mm"^3$],
  [Masa], [$m = 3.2726 times 10^(-1) "kg"$],
  [$bar(x)$], [-0.0000 mm],
  [$bar(y)$], [-0.0000 mm],
  [$bar(z)$], [-65.0000 mm],
  [$I_(x x)^(G)$], [$-3.3253 times 10^(2) " kg" dot "mm"^2$],
  [$I_(y y)^(G)$], [$-3.3253 times 10^(2) " kg" dot "mm"^2$],
  [$I_(z z)^(G)$], [$1.7612 times 10^(1) " kg" dot "mm"^2$],
  table.hline(stroke: 0.5pt),
)

= Notas sobre unidades y convenciones

- Las unidades nativas del archivo STEP se asumen en *milímetros (mm)*.
- Los momentos de inercia se reportan en kg⋅mm²; para obtener kg⋅m², multiplíquese por $10^(-6)$.
- Los productos de inercia siguen la convención de ingeniería $I_(x y) = -integral.triple x y rho dif V$ (signo negativo).
- Los momentos principales son los autovalores del tensor $[I]_G$.
