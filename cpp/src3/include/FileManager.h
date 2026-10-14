// FileManager.h — Guardado de lecturas en ficheros CSV diarios.
//
// Equivale a tu file_manager.py: crea una carpeta por día (logs/AAAA-MM-DD/)
// y, dentro, un CSV por PLC nombrado con su IP. La cabecera se escribe una sola
// vez, al crear el fichero, con los nombres de las señales.
#pragma once

#include <string>
#include <utility>
#include <vector>

// Un par (nombre de señal, valor). Usamos un vector de pares en vez de un map
// para conservar el orden de las señales tal cual vienen de la configuración
// (igual que un dict de Python, que mantiene el orden de inserción).
using Lecturas = std::vector<std::pair<std::string, float>>;

// Guarda una vuelta de lecturas en el CSV diario del PLC indicado por su IP.
void guardarVuelta(const std::string& ip, const Lecturas& valores);
