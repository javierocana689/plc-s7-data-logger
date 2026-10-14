// ConfigValidator.cpp — Validación de la configuración (versión que lanza excepción).
#include "ConfigValidator.h"

#include <stdexcept>

void validarConfig(const AppConfig& config) {
    if (config.plcs.empty()) {
        throw std::runtime_error("La configuracion no contiene ningun PLC");
    }

    for (std::size_t i = 0; i < config.plcs.size(); ++i) {
        const auto& plc = config.plcs[i];

        if (plc.ip.empty() || plc.rack < 0 || plc.slot < 0) {
            throw std::runtime_error("PLC #" + std::to_string(i) +
                                     " le faltan campos obligatorios (ip, rack o slot)");
        }
        if (plc.senales.empty()) {
            throw std::runtime_error("PLC #" + std::to_string(i) + " no tiene senales");
        }

        for (std::size_t j = 0; j < plc.senales.size(); ++j) {
            const auto& senal = plc.senales[j];
            if (senal.nombre.empty() || senal.db < 0 || senal.offset < 0 || senal.bytes <= 0) {
                throw std::runtime_error("PLC #" + std::to_string(i) + " senal #" +
                                         std::to_string(j) + " le faltan campos obligatorios");
            }
        }
    }
}
