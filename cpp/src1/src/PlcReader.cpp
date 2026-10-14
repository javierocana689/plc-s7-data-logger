// PlcReader.cpp — Implementación (versión 1): lectura señal a señal.
#include "PlcReader.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <thread>

#include <snap7.h>

#include "FileManager.h"

namespace {

// Convierte 4 bytes en formato S7 REAL (big-endian) a float.
// Equivale a snap7.util.get_real() de python-snap7.
float getRealAt(const std::uint8_t* buffer, int offset) {
    const std::uint32_t crudo =
        (static_cast<std::uint32_t>(buffer[offset])     << 24) |
        (static_cast<std::uint32_t>(buffer[offset + 1]) << 16) |
        (static_cast<std::uint32_t>(buffer[offset + 2]) << 8)  |
        (static_cast<std::uint32_t>(buffer[offset + 3]));
    float valor;
    std::memcpy(&valor, &crudo, sizeof(valor));
    return valor;
}

}  // namespace

PlcReader::PlcReader(TS7Client* cliente, const std::vector<Senal>& senales,
                     const std::string& ip, double intervalo)
    : cliente_(cliente), senales_(senales), ip_(ip), intervalo_(intervalo) {}

void PlcReader::start() {
    hilo_ = std::thread(&PlcReader::run, this);
}

void PlcReader::stop() {
    parar_.store(true);
}

void PlcReader::join() {
    if (hilo_.joinable()) hilo_.join();
}

void PlcReader::run() {
    while (!parar_.load()) {
        Lecturas valores;

        // Leemos cada señal de una en una (sin optimizar todavía).
        for (const auto& senal : senales_) {
            std::vector<std::uint8_t> datos(senal.bytes);
            const int resultado = cliente_
                ? cliente_->DBRead(senal.db, senal.offset, senal.bytes, datos.data())
                : -1;

            if (cliente_ && resultado == 0) {
                const float valor = getRealAt(datos.data(), 0);
                std::cout << senal.nombre << " " << valor << '\n';
                valores.emplace_back(senal.nombre, valor);
            } else if (!cliente_) {
                // Modo simulacion: no hay cliente real, generamos un valor de prueba.
                valores.emplace_back(senal.nombre, 20.0f);
            } else {
                std::cerr << "Error al leer del PLC\n";
            }
        }

        // Guardamos todos los datos de la vuelta de una sola vez.
        guardarVuelta(ip_, valores);

        std::this_thread::sleep_for(std::chrono::duration<double>(intervalo_));
    }
}
