// Simulador del ESP32 (sin placa): imita el webServer (puerto 80 -> 8080) y el wsServer (puerto 81)
// Uso:  node esp32-s3/sim/servidor_simulado.js     y abre http://localhost:8080/
// No necesita npm install (WebSocket implementado a mano, solo texto).

const http = require('http');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

const DATA_DIR = path.join(__dirname, '..', 'data');
const HTTP_PORT = 8080;
const WS_PORT = 81;
let wsEnPuerto81 = true; // si Windows no deja abrir el 81, el WS va por el 8080
const MODO = process.argv[2] === 'ap' ? 'ap' : 'sta'; // "ap" = simula portal de configuracion

const MIME = { '.html': 'text/html', '.css': 'text/css', '.js': 'application/javascript', '.png': 'image/png', '.webp': 'image/webp' };

// ====== HTTP: mismas rutas que Configuracion_Wifi.h ======
const httpServer = http.createServer((req, res) => {
  const url = new URL(req.url, 'http://x');
  let p = decodeURIComponent(url.pathname);
  console.log(req.method, p);

  if (p === '/') {
    res.writeHead(302, { Location: MODO === 'ap' ? '/WIFI/Interfaz_WiFi.html' : '/Scope/Interfaz_Scope.html' });
    return res.end();
  }
  if (p === '/escanear') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    return res.end(JSON.stringify(['Casa_2.4G', 'INFINITUM_ABCD', 'Laboratorio', 'Wokwi-GUEST']));
  }
  if (p === '/guardar' && req.method === 'POST') {
    let body = '';
    req.on('data', c => (body += c));
    req.on('end', () => {
      console.log('  /guardar recibio:', body);
      res.writeHead(200, { 'Content-Type': 'text/plain; charset=utf-8' });
      res.end('✅ Red guardada. Reiniciando sistema...');
    });
    return;
  }

  // manejarArchivo(): limpia prefijos igual que el firmware
  if (p.endsWith('/')) p += 'index.html';
  if (p.startsWith('/Proyecto_Incubadora')) p = p.replace('/Proyecto_Incubadora', '');
  if (p.startsWith('/data/')) p = p.slice(5);

  const file = path.join(DATA_DIR, path.normalize(p));
  if (!file.startsWith(DATA_DIR) || !fs.existsSync(file) || fs.statSync(file).isDirectory()) {
    res.writeHead(404, { 'Content-Type': 'text/plain' });
    return res.end('404: Archivo no encontrado');
  }
  res.writeHead(200, { 'Content-Type': MIME[path.extname(file)] || 'text/plain' });
  if (!wsEnPuerto81 && file.endsWith('.js')) {
    // Solo en el simulador: redirige el ws://...:81 de la pagina al puerto HTTP
    return res.end(fs.readFileSync(file, 'utf-8').replace(/:81(?!\d)/g, ':' + HTTP_PORT));
  }
  fs.createReadStream(file).pipe(res);
});
httpServer.on('upgrade', (req, sock) => manejarUpgrade(req, sock));
httpServer.listen(HTTP_PORT, () => console.log(`HTTP  -> http://localhost:${HTTP_PORT}/   (modo ${MODO})`));

// ====== WebSocket minimo (frames de texto) ======
const clientes = new Set();

function enviarTexto(sock, texto) {
  const data = Buffer.from(texto);
  let header;
  if (data.length < 126) header = Buffer.from([0x81, data.length]);
  else { header = Buffer.alloc(4); header[0] = 0x81; header[1] = 126; header.writeUInt16BE(data.length, 2); }
  sock.write(Buffer.concat([header, data]));
}

function leerFrames(sock, buf) {
  while (buf.length >= 2) {
    const op = buf[0] & 0x0f;
    let len = buf[1] & 0x7f, off = 2;
    if (len === 126) { if (buf.length < 4) break; len = buf.readUInt16BE(2); off = 4; }
    const masked = buf[1] & 0x80;
    const total = off + (masked ? 4 : 0) + len;
    if (buf.length < total) break;
    const mask = masked ? buf.slice(off, off + 4) : null;
    const payload = buf.slice(off + (masked ? 4 : 0), total);
    if (mask) for (let i = 0; i < payload.length; i++) payload[i] ^= mask[i % 4];
    buf = buf.slice(total);
    if (op === 0x8) { sock.end(); return buf; }          // close
    if (op === 0x1 && payload.toString() === 'ping') enviarTexto(sock, 'pong'); // igual que onWsEvent
  }
  return buf;
}

function manejarUpgrade(req, sock) {
  const accept = crypto.createHash('sha1')
    .update(req.headers['sec-websocket-key'] + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11').digest('base64');
  sock.write('HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n' +
             `Sec-WebSocket-Accept: ${accept}\r\n\r\n`);
  clientes.add(sock);
  console.log(`WS cliente conectado (${clientes.size})`);
  let pend = Buffer.alloc(0);
  sock.on('data', d => { pend = leerFrames(sock, Buffer.concat([pend, d])); });
  const quitar = () => { if (clientes.delete(sock)) console.log(`WS cliente desconectado (${clientes.size})`); };
  sock.on('close', quitar);
  sock.on('error', quitar);
}
const wsServer = http.createServer();
wsServer.on('upgrade', manejarUpgrade);
wsServer.on('error', e => {
  wsEnPuerto81 = false;
  console.log(`WS    -> puerto ${WS_PORT} no disponible (${e.code}); usando ws://localhost:${HTTP_PORT}`);
});
wsServer.listen(WS_PORT, () => console.log(`WS    -> ws://localhost:${WS_PORT}`));

// ====== Planta simulada (incubadora de 1er orden + PI simple) ======
// No es tu PID real: solo genera curvas creibles para probar la interfaz.
const Setpoint = 37.5, T_amb = 24, tau = 120, Kplanta = 0.08; // °C por unidad de PWM en regimen
let T = T_amb, H = 45, integral = 0, ctrl = 0;
const dt = 0.25; // igual que enviarDatosScope(): cada 250 ms
const ACELERAR = 10; // x10 para no esperar minutos

setInterval(() => {
  for (let k = 0; k < ACELERAR; k++) {
    const err = Setpoint - T;
    integral = Math.max(-2000, Math.min(2000, integral + err * dt));
    ctrl = Math.max(0, Math.min(255, 40 * err + 0.6 * integral));
    T += (dt / tau) * (Kplanta * ctrl - (T - T_amb));
    H += (dt / 60) * (60 - H);
  }
  const ruido = () => (Math.random() - 0.5) * 0.1;
  const pv = T + ruido();
  const json = JSON.stringify({
    sp: Setpoint, pv: +pv.toFixed(2), err: +(Setpoint - pv).toFixed(2),
    ctrl: +ctrl.toFixed(2), pv2: +(H + ruido() * 5).toFixed(2),
  });
  for (const c of clientes) enviarTexto(c, json); // broadcastTXT
}, dt * 1000);
