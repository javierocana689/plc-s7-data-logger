// main.cpp — Punto de entrada (versión 1).
//
// Equivalente a tu main.py original: dos listas separadas (lectores,
// clientes) — todavía no están fusionadas en una sola estructura (eso llega
// en src1_2). Sin logging: solo std::cout, igual que los print() originales.
#include <atomic>
#include <csignal>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <snap7.h>

#include "Config.h"
#include "PlcReader.h"

namespace {
std::atomic<bool> g_salir{false};
void manejadorSenal(int) { g_salir.store(true); }
}  // namespace

int main(int argc, char** argv) {
    const std::string rutaConfig = (argc > 1) ? argv[1] : "config.json";

    AppConfig config;
    try {
        config = cargarConfig(rutaConfig);
    } catch (const std::exception& e) {
        std::cerr << "Error en configuracion: " << e.what() << '\n';
        return 1;
    }

    std::signal(SIGINT, manejadorSenal);
    std::signal(SIGTERM, manejadorSenal);

    // Dos listas separadas, igual que en el main.py original de esta version.
    std::vector<std::unique_ptr<PlcReader>> lectores;
    std::vector<TS7Client*> clientes;

    for (const auto& plc : config.plcs) {
        TS7Client* cliente = nullptr;

        if (!config.modo_simulacion) {
            cliente = new TS7Client();
            const int resultado = cliente->ConnectTo(plc.ip.c_str(), plc.rack, plc.slot);
            if (resultado != 0) {
                std::cerr << "No se pudo conectar con el PLC: " << plc.ip << '\n';
                delete cliente;
                continue;
            }
            std::cout << "Conectado a " << plc.ip << '\n';
        } else {
            std::cout << "[" << plc.ip << "] Modo simulacion: no se conecta a ningun PLC\n";
        }

        auto lector = std::make_unique<PlcReader>(cliente, plc.senales, plc.ip, config.intervalo);
        lector->start();

        lectores.push_back(std::move(lector));
        clientes.push_back(cliente);
    }

    while (!g_salir.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "Interrupcion recibida\n";

    for (auto& lector : lectores) lector->stop();
    for (auto& lector : lectores) lector->join();
    for (auto* cliente : clientes) {
        if (cliente) {
            cliente->Disconnect();
            std::cout << "Desconectado del PLC\n";
            delete cliente;
        }
    }

    return 0;
}
