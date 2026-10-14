// PlcReader.cpp — Implementación (versión 1.2): señal a señal + logging.
#include "PlcReader.h"

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

    while (!parar_.load()) {
        Lecturas valores;

        for (const auto& senal : senales_) {
            if (!cliente_) {
                // Modo simulacion.
                valores.emplace_back(senal.nombre, 20.0f);
                continue;
            }

            std::vector<std::uint8_t> datos(senal.bytes);
            const int resultado = cliente_->DBRead(senal.db, senal.offset, senal.bytes, datos.data());

            if (resultado == 0) {
                const float valor = getRealAt(datos.data(), 0);
                logger().debug("[" + ip_ + "] " + senal.nombre + ": " + std::to_string(valor));
                valores.emplace_back(senal.nombre, valor);
            } else {
                logger().error("[" + ip_ + "] Error al leer " + senal.nombre);
            }
        }

        if (!valores.empty()) {
            guardarVuelta(ip_, valores);
        }

        std::this_thread::sleep_for(std::chrono::duration<double>(intervalo_));
    }

    logger().info("[" + ip_ + "] Hilo detenido");
}
