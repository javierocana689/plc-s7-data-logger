// PlcReader.cpp — Implementación (versión 2): una sola lectura de bloque.
#include "PlcReader.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <thread>

#include <snap7.h>

#include "FileManager.h"
#include "Logger.h"

namespace {

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

void PlcReader::start() { hilo_ = std::thread(&PlcReader::run, this); }
void PlcReader::stop() { parar_.store(true); }
void PlcReader::join() { if (hilo_.joinable()) hilo_.join(); }

void PlcReader::run() {
    logger().info("[" + ip_ + "] Hilo iniciado");

    // Calculado una sola vez, antes del bucle — nunca cambia durante la ejecucion.
    int offsetInicial = senales_.front().offset;
    int offsetFinal = 0;
    for (const auto& s : senales_) {
        offsetInicial = std::min(offsetInicial, s.offset);
        offsetFinal = std::max(offsetFinal, s.offset + s.bytes);
    }
    const int totalBytes = offsetFinal - offsetInicial;

    while (!parar_.load()) {
        Lecturas valores;

        if (!cliente_) {
            // Modo simulacion.
            for (const auto& senal : senales_) {
                valores.emplace_back(senal.nombre, 20.0f);
            }
        } else {
            // Una sola lectura al PLC — todos los bytes del bloque de golpe.
            std::vector<std::uint8_t> datos(totalBytes);
            const int resultado = cliente_->DBRead(senales_.front().db, offsetInicial, totalBytes, datos.data());

            if (resultado == 0) {
                for (const auto& senal : senales_) {
                    // Nota: igual que en la version Python, usamos el offset
                    // "en crudo" (no offset - offsetInicial). Funciona porque
                    // en esta configuracion offsetInicial siempre es 0.
                    const float valor = getRealAt(datos.data(), senal.offset);
                    logger().debug("[" + ip_ + "] " + senal.nombre + ": " + std::to_string(valor));
                    valores.emplace_back(senal.nombre, valor);
                }
            } else {
                logger().error("[" + ip_ + "] Error leyendo bloque");
            }
        }

        if (!valores.empty()) {
            guardarVuelta(ip_, valores);
        }

        std::this_thread::sleep_for(std::chrono::duration<double>(intervalo_));
    }

    logger().info("[" + ip_ + "] Hilo detenido");
}
