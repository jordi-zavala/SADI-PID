var WS_URL = "ws://".concat(location.hostname || '192.168.4.1', ":81");
var Muestras_Maximas = 1000;
var Tiempo_Reconexion = 3000;
var CANALES = [
    { name: 'Setpoint', color: color }
];
var head = 0;
var count = 0;
var paused = false;
var ws = null;
