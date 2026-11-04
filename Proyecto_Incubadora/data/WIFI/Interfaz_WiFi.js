// ============================================================
// CONFIGURACIÓN WIFI — CleanTECH
// ============================================================
var __spreadArray = (this && this.__spreadArray) || function (to, from, pack) {
    if (pack || arguments.length === 2) for (var i = 0, l = from.length, ar; i < l; i++) {
        if (ar || !(i in from)) {
            if (!ar) ar = Array.prototype.slice.call(from, 0, i);
            ar[i] = from[i];
        }
    }
    return to.concat(ar || Array.prototype.slice.call(from));
};
function byId(id) {
    var el = document.getElementById(id);
    if (!el) {
        throw new Error("Elemento #".concat(id, " no encontrado"));
    }
    return el;
}
var statusEl = byId('status');
var selectEl = byId('ssid');
var passwordEl = byId('password');
var btnGuardar = byId('btnGuardar');
var btnEscanear = byId('btnEscanear');
var verPassBtn = byId('verPass');
var logEl = byId('log');
var estadoModoEl = byId('estadoModo');
var numRedesEl = byId('numRedes');
var estadoRedEl = byId('estadoRed');
var modoAPEl = byId('modoAP');
var formEl = byId('configForm');
function log(mensaje, tipo) {
    var linea = document.createElement('div');
    if (tipo) {
        linea.className = tipo;
    }
    linea.innerHTML = mensaje;
    logEl.appendChild(linea);
    logEl.scrollTop = logEl.scrollHeight;
}
function setStatus(texto, clase) {
    statusEl.textContent = texto;
    statusEl.className = 'status' + (clase ? ' ' + clase : '');
}
// ------------------------------------------------------------
// Escanear redes
// ------------------------------------------------------------
var escaneando = false;
function cargarRedes() {
    if (escaneando)
        return;
    escaneando = true;
    selectEl.innerHTML = '<option value="">Buscando redes...</option>';
    setStatus('ESCANEANDO', 'wait');
    fetch('/escanear')
        .then(function (r) { return r.json(); })
        .then(function (data) {
        selectEl.innerHTML = '';
        var redes = Array.isArray(data)
            ? __spreadArray([], new Set(data), true).filter(function (s) { return Boolean(s); })
            : [];
        if (redes.length === 0) {
            selectEl.innerHTML = '<option value="">No se encontraron redes</option>';
            numRedesEl.textContent = '0';
            log('⚠️ No se detectaron redes. Presiona "Volver a escanear".', 'err');
            setStatus('SIN REDES');
            return;
        }
        redes.forEach(function (ssid) {
            var op = document.createElement('option');
            op.value = ssid;
            op.textContent = ssid;
            selectEl.appendChild(op);
        });
        numRedesEl.textContent = String(redes.length);
        estadoRedEl.textContent = String(redes.length);
        log("\uD83D\uDCF6 ".concat(redes.length, " red(es) encontradas."), 'ok');
        setStatus('LISTO', 'ok');
    })
        .catch(function () {
        selectEl.innerHTML = '<option value="">Error al escanear</option>';
        log('❌ No se pudo obtener la lista de redes.', 'err');
        setStatus('ERROR');
    })
        .finally(function () {
        escaneando = false;
    });
}
// ------------------------------------------------------------
// Guardar y conectar
// ------------------------------------------------------------
formEl.addEventListener('submit', function (e) {
    e.preventDefault();
    var ssid = selectEl.value.trim();
    var pass = passwordEl.value;
    if (!ssid) {
        log('⚠️ Selecciona una red WiFi.', 'err');
        return;
    }
    btnGuardar.disabled = true;
    btnGuardar.textContent = 'Conectando...';
    setStatus('CONECTANDO', 'wait');
    estadoModoEl.textContent = "Conectando a ".concat(ssid);
    log("\uD83D\uDD17 Conectando a \"".concat(ssid, "\"..."));
    fetch('/guardar', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: "ssid=".concat(encodeURIComponent(ssid), "&password=").concat(encodeURIComponent(pass)),
    })
        .then(function (r) { return r.text(); })
        .then(function (resp) {
        if (resp.includes('✅')) {
            log("\u2705 ".concat(resp), 'ok');
            setStatus('CONECTADO', 'ok');
            estadoModoEl.textContent = 'Red conectada correctamente';
            modoAPEl.textContent = 'STA';
            estadoRedEl.textContent = ssid.substring(0, 6);
            setTimeout(function () {
                window.location.href = '/';
            }, 3000);
        }
        else {
            log("\u274C ".concat(resp), 'err');
            setStatus('ERROR');
            estadoModoEl.textContent = 'Verifica la contraseña';
            btnGuardar.disabled = false;
            btnGuardar.textContent = 'Guardar y conectar';
        }
    })
        .catch(function () {
        log('❌ No hubo respuesta del robot. Intenta de nuevo.', 'err');
        setStatus('ERROR');
        btnGuardar.disabled = false;
        btnGuardar.textContent = 'Guardar y conectar';
    });
});
// ------------------------------------------------------------
// Mostrar / ocultar contraseña
// ------------------------------------------------------------
verPassBtn.addEventListener('click', function () {
    var mostrar = passwordEl.type === 'password';
    passwordEl.type = mostrar ? 'text' : 'password';
    verPassBtn.textContent = mostrar ? 'OCULTAR' : 'VER';
});
// ------------------------------------------------------------
// Botón escanear
// ------------------------------------------------------------
btnEscanear.addEventListener('click', function () {
    cargarRedes();
});
// ------------------------------------------------------------
// Al cargar la página
// ------------------------------------------------------------
window.addEventListener('load', function () {
    modoAPEl.textContent = 'AP';
    cargarRedes();
});
