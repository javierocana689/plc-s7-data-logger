// Config.cpp — Carga de configuración desde JSON (versión 1: sin validador separado).
#include "Config.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {

void exigirClave(const json& obj, const std::string& clave, const std::string& contexto) {
    if (!obj.contains(clave)) {
        throw std::runtime_error("Falta el campo obligatorio '" + clave + "' en " + contexto);
    }
}

}  // namespace

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

    exigirClave(datos, "plcs", "la raiz del JSON");

    int indicePlc = 0;
    for (const auto& plcJson : datos["plcs"]) {
        const std::string ctxPlc = "el PLC #" + std::to_string(indicePlc);
        exigirClave(plcJson, "ip", ctxPlc);
        exigirClave(plcJson, "senales", ctxPlc);

        PlcConfig plc;
        plc.ip = plcJson["ip"].get<std::string>();
        plc.rack = plcJson.value("rack", 0);
        plc.slot = plcJson.value("slot", 1);

        int indiceSenal = 0;
        for (const auto& senalJson : plcJson["senales"]) {
            const std::string ctxSenal = ctxPlc + " senal #" + std::to_string(indiceSenal);
            exigirClave(senalJson, "nombre", ctxSenal);
            exigirClave(senalJson, "db", ctxSenal);
            exigirClave(senalJson, "offset", ctxSenal);
            exigirClave(senalJson, "bytes", ctxSenal);

            Senal senal;
            senal.nombre = senalJson["nombre"].get<std::string>();
            senal.db = senalJson["db"].get<int>();
            senal.offset = senalJson["offset"].get<int>();
            senal.bytes = senalJson["bytes"].get<int>();
            plc.senales.push_back(senal);
            ++indiceSenal;
        }

        config.plcs.push_back(plc);
        ++indicePlc;
    }

    if (config.plcs.empty()) {
        throw std::runtime_error("La configuracion no contiene ningun PLC");
    }

    return config;
}
