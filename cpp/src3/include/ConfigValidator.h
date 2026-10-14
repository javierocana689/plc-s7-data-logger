// ConfigValidator.h — Validación de la configuración antes de arrancar.
//
// Igual que tu config_validator.py final: devuelve bool (True/False) y es
// el propio validador el que registra el error crítico, en vez de lanzar
// una excepción (eso era como funcionaba en versiones anteriores).
#pragma once

#include "Config.h"

bool validarConfig(const AppConfig& config);
