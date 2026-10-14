// Config.cpp — Carga de configuración desde JSON (permisiva: no valida).
#include "Config.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

AppConfig cargarConfig(const std::string& ruta) {
    std::ifstream fichero(ruta);
    if (!fichero.is_open()) {
        throw std::runtime_error("No se pudo abrir el fichero de configuracion: " + ruta);
    }

    json datos;
    try {
        fichero >> datos;
    } catch (const json::parse_error& e) {
        throw std::runtime_error(std::string("JSON invalido: ") + e.what());
    }

    AppConfig config;
    config.intervalo = datos.value("intervalo", 1.0);
    config.intervalo_reconexion = datos.value("intervalo_reconexion", 5.0);
    config.modo_simulacion = datos.value("modo_simulacion", false);

    if (!datos.contains("plcs") || !datos["plcs"].is_array()) {
        return config;  // Se deja vacio; ConfigValidator lo detectara.
    }

    for (const auto& plcJson : datos["plcs"]) {
        PlcConfig plc;
        plc.ip = plcJson.value("ip", "");
        plc.rack = plcJson.value("rack", -1);
        plc.slot = plcJson.value("slot", -1);

        if (plcJson.contains("senales") && plcJson["senales"].is_array()) {
            for (const auto& senalJson : plcJson["senales"]) {
                Senal senal;
                senal.nombre = senalJson.value("nombre", "");
                senal.db = senalJson.value("db", -1);
                senal.offset = senalJson.value("offset", -1);
                senal.bytes = senalJson.value("bytes", 0);
                plc.senales.push_back(senal);
            }
        }

        config.plcs.push_back(plc);
    }

    return config;
}
