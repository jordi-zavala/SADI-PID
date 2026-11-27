const WS_URL: string = `ws://${location.hostname || '192.168.4.1'}:81`;
const Muestras_Maximas: number = 1000;
const Tiempo_Reconexion: number =3000;


interface Canales {
    name: string;
    color: string;
    buf : Float32Array;
    visible: boolean;
}

const CANALES: Canales[] = [
    {name:'Setpoint', color}
]

let head: number=0;
let count: number =0;
let paused: boolean = false;
let ws: WebSocket | null = null;

function byId <T 