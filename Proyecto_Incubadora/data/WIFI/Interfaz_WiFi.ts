// ============================================================
// CONFIGURACIÓN WIFI — CleanTECH
// ============================================================

type LogTipo = 'ok' | 'err';

function byId<T extends HTMLElement>(id: string): T {
    const el = document.getElementById(id);
    if (!el) {
        throw new Error(`Elemento #${id} no encontrado`);
    }
    return el as T;
}

const statusEl = byId<HTMLDivElement>('status');
const selectEl = byId<HTMLSelectElement>('ssid');
const passwordEl = byId<HTMLInputElement>('password');
const btnGuardar = byId<HTMLButtonElement>('btnGuardar');
const btnEscanear = byId<HTMLButtonElement>('btnEscanear');
const verPassBtn = byId<HTMLButtonElement>('verPass');
const logEl = byId<HTMLDivElement>('log');
const estadoModoEl = byId<HTMLDivElement>('estadoModo');
const numRedesEl = byId<HTMLDivElement>('numRedes');
const estadoRedEl = byId<HTMLDivElement>('estadoRed');
const modoAPEl = byId<HTMLDivElement>('modoAP');
const formEl = byId<HTMLFormElement>('configForm');

function log(mensaje: string, tipo?: LogTipo): void {
    const linea = document.createElement('div');
    if (tipo) {
        linea.className = tipo;
    }
    linea.innerHTML = mensaje;
    logEl.appendChild(linea);
    logEl.scrollTop = logEl.scrollHeight;
}

function setStatus(texto: string, clase?: string): void {
    statusEl.textContent = texto;
    statusEl.className = 'status' + (clase ? ' ' + clase : '');
}

// ------------------------------------------------------------
// Escanear redes
// ------------------------------------------------------------
let escaneando = false;

function cargarRedes(): void {
    if (escaneando) return;
    escaneando = true;

    selectEl.innerHTML = '<option value="">Buscando redes...</option>';
    setStatus('ESCANEANDO', 'wait');

    fetch('/escanear')
        .then((r: Response) => r.json())
        .then((data: unknown) => {
            selectEl.innerHTML = '';

            const redes: string[] = Array.isArray(data)
                ? [...new Set(data as string[])].filter((s) => Boolean(s))
                : [];

            if (redes.length === 0) {
                selectEl.innerHTML = '<option value="">No se encontraron redes</option>';
                numRedesEl.textContent = '0';
                log('⚠️ No se detectaron redes. Presiona "Volver a escanear".', 'err');
                setStatus('SIN REDES');
                return;
            }

            redes.forEach((ssid) => {
                const op = document.createElement('option');
                op.value = ssid;
                op.textContent = ssid;
                selectEl.appendChild(op);
            });

            numRedesEl.textContent = String(redes.length);
            estadoRedEl.textContent = String(redes.length);
            log(`📶 ${redes.length} red(es) encontradas.`, 'ok');
            setStatus('LISTO', 'ok');
        })
        .catch(() => {
            selectEl.innerHTML = '<option value="">Error al escanear</option>';
            log('❌ No se pudo obtener la lista de redes.', 'err');
            setStatus('ERROR');
        })
        .finally(() => {
            escaneando = false;
        });
}

// ------------------------------------------------------------
// Guardar y conectar
// ------------------------------------------------------------
formEl.addEventListener('submit', (e: Event) => {
    e.preventDefault();

    const ssid = selectEl.value.trim();
    const pass = passwordEl.value;

    if (!ssid) {
        log('⚠️ Selecciona una red WiFi.', 'err');
        return;
    }

    btnGuardar.disabled = true;
    btnGuardar.textContent = 'Conectando...';
    setStatus('CONECTANDO', 'wait');
    estadoModoEl.textContent = `Conectando a ${ssid}`;
    log(`🔗 Conectando a "${ssid}"...`);

    fetch('/guardar', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `ssid=${encodeURIComponent(ssid)}&password=${encodeURIComponent(pass)}`,
    })
        .then((r: Response) => r.text())
        .then((resp: string) => {
            if (resp.includes('✅')) {
                log(`✅ ${resp}`, 'ok');
                setStatus('CONECTADO', 'ok');
                estadoModoEl.textContent = 'Red conectada correctamente';
                modoAPEl.textContent = 'STA';
                estadoRedEl.textContent = ssid.substring(0, 6);
                setTimeout(() => {
                    window.location.href = '/';
                }, 3000);
            } else {
                log(`❌ ${resp}`, 'err');
                setStatus('ERROR');
                estadoModoEl.textContent = 'Verifica la contraseña';
                btnGuardar.disabled = false;
                btnGuardar.textContent = 'Guardar y conectar';
            }
        })
        .catch(() => {
            log('❌ No hubo respuesta del robot. Intenta de nuevo.', 'err');
            setStatus('ERROR');
            btnGuardar.disabled = false;
            btnGuardar.textContent = 'Guardar y conectar';
        });
});

// ------------------------------------------------------------
// Mostrar / ocultar contraseña
// ------------------------------------------------------------
verPassBtn.addEventListener('click', () => {
    const mostrar = passwordEl.type === 'password';
    passwordEl.type = mostrar ? 'text' : 'password';
    verPassBtn.textContent = mostrar ? 'OCULTAR' : 'VER';
});

// ------------------------------------------------------------
// Botón escanear
// ------------------------------------------------------------
btnEscanear.addEventListener('click', () => {
    cargarRedes();
});

// ------------------------------------------------------------
// Al cargar la página
// ------------------------------------------------------------
window.addEventListener('load', () => {
    modoAPEl.textContent = 'AP';
    cargarRedes();
});