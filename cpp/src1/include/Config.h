// Config.h — Estructuras de configuración y carga desde JSON.
//
// Equivalente a tu config.py: aquí definimos la forma de los datos.
// En esta primera versión, igual que en Python, la carga ya exige los
// campos obligatorios (no hay todavía un config_validator separado en uso).
#pragma once

#include <string>
#include <vector>

struct Senal {
    std::string nombre;
    int db = 0;
    int offset = 0;
    int bytes = 0;
};

struct PlcConfig {
    std::string ip;
    int rack = 0;
    int slot = 1;
    std::vector<Senal> senales;
};

struct AppConfig {
    std::vector<PlcConfig> plcs;
    double intervalo = 1.0;
    double intervalo_reconexion = 5.0;  // Definido pero aún sin usarse (como en Python)
    bool modo_simulacion = false;       // Para poder probar sin PLC real
};

// Carga la configuración desde un fichero JSON. Lanza std::runtime_error
// si el fichero no existe, el JSON es inválido o falta algún campo.
AppConfig cargarConfig(const std::string& ruta);
