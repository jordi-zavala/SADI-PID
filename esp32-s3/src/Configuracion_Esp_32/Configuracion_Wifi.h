#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <WebSocketsServer.h> // <-- NUEVA LIBRERIA NECESARIA
#include "Variables_del_sistema.h"
#include "Configuracion_Del_sensor.h"
#include "../Control_incubadora/Variables_PID.h"

bool WiFi_Conectado = false;
const byte DNS_PORT = 53;

// Credenciales del punto de acceso (Modo AP para configuración)
const char* AP_SSID = "SADI_Config";
const char* AP_PASS = "12345678";

DNSServer dnsServer;
WebServer webServer(80);
WebSocketsServer wsServer(81); // Servidor WebSocket para el Scope
Preferences preferences;

unsigned long ultimoWS = 0; // Timer para el Scope

// ============================================================
// FUNCIONES DEL SERVIDOR WEB (WiFi y Archivos)
// ============================================================
void manejarEscanear() {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; ++i) {
        if (i > 0) json += ",";
        json += "\"" + WiFi.SSID(i) + "\"";
    }
    json += "]";
    webServer.send(200, "application/json", json);
}

void manejarGuardar() {
    String ssid = webServer.arg("ssid");
    String password = webServer.arg("password");
    
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
    
    webServer.send(200, "text/plain", "✅ Red guardada. Reiniciando sistema...");
    delay(1000);
    ESP.restart();
}

void manejarArchivo() {
    String path = webServer.uri();
    if (path.endsWith("/")) path += "index.html";
    
    // Mapeo de tipos MIME
    String contentType = "text/plain";
    if (path.endsWith(".html")) contentType = "text/html";
    else if (path.endsWith(".css")) contentType = "text/css";
    else if (path.endsWith(".js")) contentType = "application/javascript";
    else if (path.endsWith(".png")) contentType = "image/png";
    else if (path.endsWith(".webp")) contentType = "image/webp";

    // Si la ruta incluye "/Proyecto_Incubadora/data/..." la limpiamos
    if (path.startsWith("/Proyecto_Incubadora")) {
        path.replace("/Proyecto_Incubadora", "");
    }

    // FIX: uploadfs sube el contenido de data/ a la raíz, así que "/data/WIFI/x" -> "/WIFI/x"
    if (path.startsWith("/data/")) path.remove(0, 5);

    if (LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        webServer.streamFile(file, contentType);
        file.close();
    } else {
        webServer.send(404, "text/plain", "404: Archivo no encontrado");
    }
}

void manejarRaiz() {
    // Redirige a la interfaz WiFi si no hay conexión, o al Scope si ya está conectado
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        webServer.sendHeader("Location", "/WIFI/Interfaz_WiFi.html");
        webServer.send(302, "text/plain", "");
    } else {
        webServer.sendHeader("Location", "/Scope/Interfaz_Scope.html");
        webServer.send(302, "text/plain", "");
    }
}

// ============================================================
// FUNCIONES WEBSOCKET (Para la interfaz Scope)
// ============================================================
void onWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    // Aquí puedes manejar comandos que vengan desde el Scope (ej. Pausar)
    if (type == WStype_TEXT) {
        String msg = String((char*)payload);
        if (msg == "ping") {
            wsServer.sendTXT(num, "pong");
        }
    }
}

void enviarDatosScope() {
    if (millis() - ultimoWS > 250) { // Enviar datos cada 250ms
        ultimoWS = millis();
        
        // Construir el JSON con los datos exactos que espera Interfaz_Scope.ts
        String json = "{";
        json += "\"sp\":" + String(Setpoint) + ",";
        json += "\"pv\":" + String(Point_Value_Temperatura_Incubadora) + ",";
        json += "\"err\":" + String(Setpoint - Point_Value_Temperatura_Incubadora) + ",";
        json += "\"ctrl\":" + String(Y0_PID_Temperatura[0]) + ",";
        json += "\"pv2\":" + String(Point_Value_Humedad_Incubadora);
        json += "}";
        
        wsServer.broadcastTXT(json);
    }
}