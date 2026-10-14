// FileManager.h — Guardado de lecturas en ficheros CSV diarios.
//
// Desde esta versión usamos logging y escribimos con BOM UTF-8 (equivalente
// a utf-8-sig en Python) para que Excel muestre bien los acentos.
#pragma once

#include <string>
#include <utility>
#include <vector>

using Lecturas = std::vector<std::pair<std::string, float>>;

void guardarVuelta(const std::string& ip, const Lecturas& valores);
