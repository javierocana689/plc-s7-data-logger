// FileManager.h — Guardado de lecturas en ficheros CSV diarios.
//
// Equivalente a tu file_manager.py de esta primera versión: aún sin logging,
// codificación UTF-8 simple (sin BOM; eso llega en la siguiente iteración).
#pragma once

#include <string>
#include <utility>
#include <vector>

using Lecturas = std::vector<std::pair<std::string, float>>;

void guardarVuelta(const std::string& ip, const Lecturas& valores);
