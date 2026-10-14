// Config.h — Estructuras de configuración y carga desde JSON.
//
// A partir de esta versión, cargarConfig() es permisiva: no exige los
// campos (usa valores por defecto si faltan). La comprobación real de que
// todo esté bien ahora vive en ConfigValidator, igual que en tu
// config_validator.py separado de config.py.
#pragma once

#include <string>
#include <vector>

struct Senal {
    std::string nombre;  // Vacío si falta en el JSON
    int db = -1;         // -1 si falta (valor inválido, lo detecta el validador)
    int offset = -1;
    int bytes = 0;
};

struct PlcConfig {
    std::string ip;      // Vacío si falta
    int rack = -1;
    int slot = -1;
    std::vector<Senal> senales;
};

struct AppConfig {
    std::vector<PlcConfig> plcs;
    double intervalo = 1.0;
    double intervalo_reconexion = 5.0;
    bool modo_simulacion = false;
};

// Carga la configuración desde un fichero JSON. Lanza std::runtime_error
// solo si el fichero no se puede abrir o el JSON está mal formado;
// los campos obligatorios se validan por separado (ver ConfigValidator.h).
AppConfig cargarConfig(const std::string& ruta);
