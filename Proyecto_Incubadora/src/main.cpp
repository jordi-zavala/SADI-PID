// =====================================================================
// SADI · Simulación lazo abierto (CSV) / lazo cerrado (PID + TF discreta)
// =====================================================================
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <math.h>

// ---------------- CONFIG ----------------
const char* AP_SSID = "SADI_Sim";
const char* AP_PASS = "";
constexpr int WS_PORT = 81;

// Modelo FOPDT (identificad en main2.cpp)
constexpr double K_TEMP   = 68.50;
constexpr double TAU_TEMP = 172.0;
constexpr double L_TEMP   = 4.0;
constexpr double TS       = 2.0;

// TF discreta:  ΔT[k] = TF_A·ΔT[k-1] + TF_B·u[k-1-Nd]
constexpr double TF_A = 0.98843462;   // exp(-Ts/Tau)
constexpr double TF_B = 0.79216532;   // K*(1-TF_A)
constexpr int    ND   = 2;            // 4s / 2s

constexpr double T_AMB      = 23.6;
constexpr double SP_DEFAULT = 37.5;

// PID IMC
constexpr double KP  = 0.0739;
constexpr double TI  = 172.0;
constexpr double TD  = 0.0;
constexpr double KPD = KP;
constexpr double KID = KP * TS / TI;
constexpr double KDD = KP * TD / TS;

// Ritmos
constexpr int PERIODO_SIM_MS = 100;   // 20× velocidad
constexpr int PERIODO_WS_MS  = 100;

// ---------------- ESTADO ----------------
enum Modo { MODO_CL, MODO_OL, MODO_MAN };
Modo modo = MODO_CL;

double sp = SP_DEFAULT, pv = T_AMB;
double uCtrl = 0.0, uManual = 0.0, simTime = 0.0;

// Planta y PID
double uBuf[8] = {0};  int uIdx = 0;
double eBuf[3]    = {0, 0, 0};
double uBufPid[2] = {0, 0};
volatile bool reset_flag = false;

// ---------------- PLANTA ----------------
double planta_step(double u) {
  uBuf[uIdx] = u;
  int idxDel = (uIdx - ND + 8) % 8;
  double uDel = uBuf[idxDel];
  uIdx = (uIdx + 1) % 8;

  double dTprev = pv - T_AMB;
  double dTnew  = TF_A * dTprev + TF_B * uDel;
  return T_AMB + dTnew;
}

// ---------------- PID ----------------
double pid_step(double setpoint, double medicion) {
  eBuf[0] = setpoint - medicion;
  double uNew = eBuf[0] * (KPD + KID + KDD)
              - eBuf[1] * (KPD + 2 * KDD)
              + eBuf[2] * KDD
              + uBufPid[0];
  if (uNew > 1.0) uNew = 1.0;
  if (uNew < 0.0) uNew = 0.0; // Calentador no puede enfriar
  eBuf[2] = eBuf[1];  eBuf[1] = eBuf[0];
  uBufPid[1] = uBufPid[0];  uBufPid[0] = uNew;
  return uNew;
}

// ---------------- CSV ----------------
const int CSV_N = 46;
const float CSV_T[CSV_N] = {
  0,40,80,120,160,200,240,280,320,360,400,440,480,520,560,600,
  640,680,720,760,800,840,880,920,960,1000,1040,1080,1120,1160,
  1200,1240,1280,1320,1360,1400,1440,1480,1520,1560,1600,1640,
  1680,1720,1760,1800
};
const float CSV_TE[CSV_N] = {
  23.6,25.2,26.4,27.3,27.7,28.5,28.9,29.2,29.5,29.7,29.9,30.1,30.2,30.3,30.4,30.5,
  30.5,30.5,30.6,30.7,30.7,30.8,30.8,30.8,30.8,30.8,30.8,30.8,30.8,30.8,
  30.8,30.8,30.8,30.8,30.8,30.9,30.9,30.9,30.9,30.9,30.9,30.9,
  30.9,30.9,30.9,30.9
};
double csv_at(double t) {
  if (t <= 0) return CSV_TE[0];
  if (t >= CSV_T[CSV_N - 1]) return CSV_TE[CSV_N - 1];
  for (int i = 0; i < CSV_N - 1; i++) {
    if (t < CSV_T[i + 1]) {
      float f = (t - CSV_T[i]) / (CSV_T[i + 1] - CSV_T[i]);
      return CSV_TE[i] + f * (CSV_TE[i + 1] - CSV_TE[i]);
    }
  }
  return CSV_TE[CSV_N - 1];
}

// ---------------- TICK ----------------
void sim_tick() {
  double u;
  switch (modo) {
    case MODO_CL:  u = pid_step(sp, pv); break;
    case MODO_OL:  u = 0.1;              break;
    case MODO_MAN: u = uManual;          break;
    default: u = 0;
  }
  uCtrl = u;

  if (modo == MODO_OL) pv = csv_at(simTime);
  else                 pv = planta_step(u);

  simTime += TS;
}

void reset_sim() {
  pv = T_AMB;  sp = SP_DEFAULT;  simTime = 0;
  uCtrl = 0;   uManual = 0;
  for (int i = 0; i < 8; i++) uBuf[i] = 0;
  uIdx = 0;
  for (int i = 0; i < 3; i++) eBuf[i] = 0;
  uBufPid[0] = uBufPid[1] = 0;
  reset_flag = true;
}

// ---------------- WEB + WS ----------------
WebServer http(80);
WebSocketsServer ws(WS_PORT);

void onWs(uint8_t n, WStype_t t, uint8_t* pl, size_t len) {
  if (t != WStype_TEXT) return;
  StaticJsonDocument<256> d;
  if (deserializeJson(d, (char*)pl)) return;
  const char* ty = d["type"] | "";

  if (!strcmp(ty, "mode")) {
    const char* m = d["mode"] | "";
    if      (!strcmp(m, "cl"))  modo = MODO_CL;
    else if (!strcmp(m, "ol"))  modo = MODO_OL;
    else if (!strcmp(m, "man")) modo = MODO_MAN;
    reset_sim();
  }
  else if (!strcmp(ty, "reset")) reset_sim();
  else if (!strcmp(ty, "uman"))  uManual = d["value"] | 0.0;
  else if (!strcmp(ty, "sp"))    sp      = d["value"] | 37.5;
}

// ---------------- HTML/CSS/JS EMBEBIDO ----------------
const char HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="es"><head><meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SADI · Simulación PID + TF</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;font-family:system-ui,sans-serif}
body{background:#0f172a;color:#e2e8f0;min-height:100vh;padding:16px}
.app{max-width:1100px;margin:0 auto;display:flex;flex-direction:column;gap:10px}
header{display:flex;justify-content:space-between;align-items:center;padding:12px 16px;background:#1e293b;border-radius:14px}
h1{font-size:1.05rem;color:#38bdf8}
.status{display:flex;gap:8px;align-items:center;font-size:.8rem;color:#94a3b8}
.dot{width:10px;height:10px;border-radius:50%;background:#ef4444;transition:.2s}
.dot.on{background:#22c55e}
.toolbar{display:flex;flex-wrap:wrap;gap:10px;padding:10px 12px;background:#1e293b;border-radius:12px;align-items:center}
.btn{padding:8px 14px;border-radius:10px;border:none;background:#334155;color:#e2e8f0;font-size:.78rem;font-weight:600;cursor:pointer}
.btn:hover{background:#475569}
.btn.active{background:#0284c7}
.btn.danger{background:#b91c1c}
.chip{display:flex;align-items:center;gap:6px;padding:6px 12px;border-radius:20px;background:#0f172a;font-size:.78rem;border:1px solid #334155}
.sw{width:12px;height:12px;border-radius:3px}
.scope-wrap{background:#1e293b;border-radius:14px;padding:12px;position:relative}
canvas{width:100%;height:360px;display:block;border-radius:8px;background:#060b14}
.stats{display:grid;grid-template-columns:repeat(4,1fr);gap:10px}
.stat{background:#1e293b;border-radius:10px;padding:10px 14px}
.stat .lbl{font-size:.7rem;color:#94a3b8;text-transform:uppercase;letter-spacing:.5px}
.stat .val{font-size:1.15rem;font-weight:700;margin-top:4px;font-variant-numeric:tabular-nums}
.stat.sp .val{color:#fbbf24}.stat.pv .val{color:#38bdf8}
.stat.err .val{color:#ef4444}.stat.ctrl .val{color:#34d399}
.slider{display:flex;gap:8px;align-items:center;font-size:.78rem;color:#94a3b8}
input[type=range]{width:110px}
</style></head><body><div class="app">
<header><h1>SADI · Simulación TF discreta + PID</h1>
<div class="status"><div class="dot" id="dot"></div><span id="stTxt">Desconectado</span></div></header>
<div class="toolbar">
  <button class="btn active" data-mode="cl" id="bCL">Lazo Cerrado (PID)</button>
  <button class="btn" data-mode="ol" id="bOL">Lazo Abierto (CSV)</button>
  <button class="btn" data-mode="man" id="bMAN">Manual</button>
  <button class="btn danger" id="bReset">Reset</button>
  <div class="chip"><div class="sw" style="background:#fbbf24"></div>SP</div>
  <div class="chip"><div class="sw" style="background:#38bdf8"></div>PV</div>
  <div class="chip"><div class="sw" style="background:#34d399"></div>u</div>
  <div class="slider">u_manual<input type="range" id="uMan" min="0" max="1" step="0.01" value="0"></div>
  <div class="slider">SP<input type="range" id="spIn" min="20" max="60" step="0.5" value="37.5"></div>
</div>
<div class="scope-wrap"><canvas id="cv"></canvas></div>
<div class="stats">
  <div class="stat sp"><div class="lbl">Setpoint</div><div class="val" id="vSP">--</div></div>
  <div class="stat pv"><div class="lbl">Temperatura</div><div class="val" id="vPV">--</div></div>
  <div class="stat err"><div class="lbl">Error</div><div class="val" id="vErr">--</div></div>
  <div class="stat ctrl"><div class="lbl">Control u</div><div class="val" id="vU">--</div></div>
</div></div>
<script>
let ws=null, samples=[], MAX=900, lastT=-1;
const cv=document.getElementById('cv'), ctx=cv.getContext('2d');
function fit(){const r=cv.getBoundingClientRect(),d=window.devicePixelRatio||1;
  cv.width=r.width*d; cv.height=r.height*d; ctx.setTransform(d,0,0,d,0,0);}
window.addEventListener('resize',fit); fit();

function draw(){
  const W=cv.width/(window.devicePixelRatio||1), H=cv.height/(window.devicePixelRatio||1);
  ctx.fillStyle='#060b14'; ctx.fillRect(0,0,W,H);
  
  // Cuadrícula y Ejes
  ctx.strokeStyle='#1e293b'; ctx.lineWidth=1;
  ctx.fillStyle='#64748b'; ctx.font='10px monospace'; ctx.textAlign='right';
  const yMin=20, yMax=60, ySteps=4;
  for(let i=0;i<=ySteps;i++){
    const val = yMin + (yMax-yMin)*(i/ySteps);
    const y = H - (i/ySteps)*H;
    ctx.beginPath(); ctx.moveTo(40,y); ctx.lineTo(W,y); ctx.stroke();
    ctx.fillText(val.toFixed(0)+'°', 35, y+3);
  }
  
  if(samples.length<2)return;
  const t0=samples[0].t, t1=samples[samples.length-1].t, span=(t1-t0)||1;
  const X=t=>40+((t-t0)/span)*(W-45); 
  const Y=v=>H-((v-yMin)/(yMax-yMin))*H;
  
  function plot(col,scale){
    ctx.strokeStyle=col; ctx.lineWidth=2; ctx.beginPath();
    samples.forEach((s,i)=>{
      const x=X(s.t), y=Y(scale(s));
      if(i===0) ctx.moveTo(x,y);
      else ctx.lineTo(x,y);
    });
    ctx.stroke();
  }
  
  plot('#fbbf24', s=>s.sp);
  plot('#38bdf8', s=>s.pv);
  plot('#34d399', s=>20+s.u*8);   // u mapeada de 0 a 1 en el rango de 20 a 28 °C
}

function connect(){
  ws=new WebSocket('ws://'+(location.hostname||'192.168.4.1')+':81');
  ws.onopen=()=>{document.getElementById('dot').classList.add('on');
                 document.getElementById('stTxt').textContent='Conectado';};
  ws.onclose=()=>{document.getElementById('dot').classList.remove('on');
                  document.getElementById('stTxt').textContent='Desconectado';
                  setTimeout(connect,3000);};
  ws.onmessage=e=>{
    try{
      const m=JSON.parse(e.data);
      if(m.type!=='state') return;
      const p=m.payload;
      
      // Si el ESP32 reinició la simulación (t=0), limpiamos la pantalla
      if(p.t < lastT) { samples=[]; }
      lastT = p.t;

      document.getElementById('vSP').textContent=p.sp.toFixed(2)+' °C';
      document.getElementById('vPV').textContent=p.pv.toFixed(2)+' °C';
      document.getElementById('vErr').textContent=p.err.toFixed(2);
      document.getElementById('vU').textContent=p.u.toFixed(3);
      
      samples.push({t:p.t,sp:p.sp,pv:p.pv,u:p.u});
      if(samples.length>MAX) samples.shift();
      draw();
    }catch(_){}
  };
}
connect();

document.querySelectorAll('[data-mode]').forEach(b=>{
  b.onclick=()=>{
    document.querySelectorAll('[data-mode]').forEach(x=>x.classList.remove('active'));
    b.classList.add('active');
    samples=[]; // Limpiar gráfica al cambiar de modo
    if(ws&&ws.readyState===1) ws.send(JSON.stringify({type:'mode',mode:b.dataset.mode}));
  };
});
document.getElementById('bReset').onclick=()=>{
  samples=[]; // Limpiar gráfica al resetear
  if(ws&&ws.readyState===1) ws.send('{"type":"reset"}');
};
document.getElementById('uMan').oninput=e=>{if(ws&&ws.readyState===1) ws.send(JSON.stringify({type:'uman',value:parseFloat(e.target.value)}));};
document.getElementById('spIn').oninput=e=>{if(ws&&ws.readyState===1) ws.send(JSON.stringify({type:'sp',value:parseFloat(e.target.value)}));};
</script></body></html>
)HTML";

// ---------------- SETUP / LOOP ----------------
unsigned long ultSim = 0, ultWS = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("URL: http://"); Serial.println(WiFi.softAPIP());

  http.on("/", [](){ http.send_P(200, "text/html", HTML); });
  http.begin();
  ws.begin();
  ws.onEvent(onWs);

  ultSim = ultWS = millis();
  reset_sim();
}

void loop() {
  http.handleClient();
  ws.loop();

  unsigned long ahora = millis();

  if (ahora - ultSim >= PERIODO_SIM_MS) {
    ultSim = ahora;
    sim_tick();
  }

  if (ahora - ultWS >= PERIODO_WS_MS) {
    ultWS = ahora;
    StaticJsonDocument<200> d;
    d["type"] = "state";
    JsonObject p = d.createNestedObject("payload");
    p["t"]   = (float)simTime;
    p["sp"]  = (float)sp;
    p["pv"]  = (float)pv;
    p["err"] = (float)(sp - pv);
    p["u"]   = (float)uCtrl;
    String s; serializeJson(d, s);
    ws.broadcastTXT(s);
  }
}