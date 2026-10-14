// ConfigValidator.h — Validación de la configuración antes de arrancar.
//
// Equivalente a tu config_validator.py de esta versión: lanza una
// excepción si falta algún campo obligatorio.
#pragma once

#include "Config.h"

// Lanza std::runtime_error si algún PLC o señal tiene campos inválidos.
void validarConfig(const AppConfig& config);
