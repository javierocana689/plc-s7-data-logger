// Config.h — Estructuras de configuración y carga desde JSON.
//
// Equivale a tu config.py de Python: aquí definimos la forma de los datos
// (qué campos tiene un PLC y una señal) y una función para cargarlos desde
// un fichero config.json externo.
#pragma once

#include <string>
#include <vector>

// Una señal individual a leer dentro de un bloque de datos (DB) del PLC.
struct Senal {
    std::string nombre;   // Nombre legible, p. ej. "Temperatura"
    int db = 0;           // Número de bloque de datos (DB) en el PLC
    int offset = 0;       // Desplazamiento en bytes dentro del DB
    int bytes = 0;        // Tamaño en bytes del dato (4 para un REAL)
};

// Configuración de un PLC concreto.
struct PlcConfig {
    std::string ip;             // IP del PLC
    int rack = 0;               // Rack (normalmente 0 en S7-1200)
    int slot = 1;               // Slot (normalmente 1 en S7-1200)
    std::string password;       // Contraseña de Full access (opcional)
    std::vector<Senal> senales; // Señales a leer de este PLC
};

// Configuración global de la aplicación.
struct AppConfig {
    std::vector<PlcConfig> plcs;          // Lista de PLCs
    double intervalo = 1.0;               // Segundos entre lecturas (polling)
    double intervalo_reconexion = 5.0;    // Segundos entre reintentos de conexión
    bool modo_simulacion = false;         // Si true, genera datos falsos sin PLC real
};

// Carga y valida la configuración desde un fichero JSON.
// Lanza std::runtime_error si el fichero no existe, el JSON es inválido
// o falta algún campo obligatorio (equivale a tu config_validator.py).
AppConfig cargarConfig(const std::string& ruta);
