// ConfigValidator.cpp — Validación de la configuración (versión final: bool + logger).
#include "ConfigValidator.h"

#include "Logger.h"

bool validarConfig(const AppConfig& config) {
    if (config.plcs.empty()) {
        logger().critical("La configuracion no contiene ningun PLC");
        return false;
    }

    for (std::size_t i = 0; i < config.plcs.size(); ++i) {
        const auto& plc = config.plcs[i];

        if (plc.ip.empty() || plc.rack < 0 || plc.slot < 0) {
            logger().critical("PLC #" + std::to_string(i) +
                              " le faltan campos obligatorios (ip, rack o slot)");
            return false;
        }
        if (plc.senales.empty()) {
            logger().critical("PLC #" + std::to_string(i) + " no tiene senales");
            return false;
        }

        for (std::size_t j = 0; j < plc.senales.size(); ++j) {
            const auto& senal = plc.senales[j];
            if (senal.nombre.empty() || senal.db < 0 || senal.offset < 0 || senal.bytes <= 0) {
                logger().critical("PLC #" + std::to_string(i) + " senal #" +
                                  std::to_string(j) + " le faltan campos obligatorios");
                return false;
            }
        }
    }

    return true;
}
