// Config.h — Estructuras de configuración y carga desde JSON.
//
// Igual que en las versiones anteriores, cargarConfig() es permisiva.
// La validación real vive en ConfigValidator (ver más abajo), que en esta
// versión devuelve bool en vez de lanzar excepción — igual que tu
// config_validator.py final, que pasó de `raise ValueError` a
// `return True/False` con logger.critical.
#pragma once

#include <string>
#include <vector>

struct Senal {
    std::string nombre;
    int db = -1;
    int offset = -1;
    int bytes = 0;
};

struct PlcConfig {
    std::string ip;
    int rack = -1;
    int slot = -1;
    std::string password;
    std::vector<Senal> senales;
};

struct AppConfig {
    std::vector<PlcConfig> plcs;
    double intervalo = 1.0;
    double intervalo_reconexion = 5.0;
    bool modo_simulacion = false;
};

AppConfig cargarConfig(const std::string& ruta);
