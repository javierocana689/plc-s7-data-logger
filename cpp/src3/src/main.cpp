// main.cpp — Punto de entrada del sistema de adquisición de datos PLC.
//
// Equivale a tu main.py: configura el logging, carga y valida la configuración,
// lanza un hilo por PLC y gestiona el apagado ordenado con Ctrl+C.
#include <atomic>
#include <csignal>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "Config.h"
#include "ConfigValidator.h"
#include "FileManager.h"
#include "Logger.h"
#include "PlcReader.h"

namespace {

// Bandera global que el manejador de Ctrl+C pone a true para pedir el cierre.
std::atomic<bool> g_salir{false};

void manejadorSenal(int) {
    g_salir.store(true);
}

}  // namespace

int main(int argc, char** argv) {
    // La ruta del config puede pasarse como argumento; por defecto "config.json".
    const std::string rutaConfig = (argc > 1) ? argv[1] : "config.json";

    // Logging a consola (INFO+) y a fichero rotativo (todo).
    logger().configurar("sistema_plc.log", Nivel::INFO);
    logger().info("Iniciando sistema de adquisicion de datos");

    // Cargar y validar la configuración.
    AppConfig config;
    try {
        config = cargarConfig(rutaConfig);
    } catch (const std::exception& e) {
        logger().critical(std::string("Error en configuracion: ") + e.what());
        return 1;
    }

    if (!validarConfig(config)) {
        logger().critical("Error en configuracion. Saliendo.");
        return 1;
    }

    if (config.modo_simulacion) {
        logger().info("MODO SIMULACION ACTIVO: se generaran datos de prueba sin PLC");
    }

    // Capturar Ctrl+C para un apagado limpio.
    std::signal(SIGINT, manejadorSenal);
    std::signal(SIGTERM, manejadorSenal);

    // Un lector autónomo por PLC (usamos unique_ptr porque PlcReader no es copiable).
    std::vector<std::unique_ptr<PlcReader>> lectores;
    for (const auto& plc : config.plcs) {
        auto lector = std::make_unique<PlcReader>(
            plc, config.intervalo, config.intervalo_reconexion, config.modo_simulacion);
        lector->start();
        lectores.push_back(std::move(lector));
    }

    if (lectores.empty()) {
        logger().critical("No se creo ningun lector. Saliendo.");
        return 1;
    }

    // Mantener el programa vivo hasta recibir Ctrl+C.
    while (!g_salir.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    logger().info("Interrupcion recibida. Cerrando sistema...");

    // Apagado ordenado: primero pedimos a todos que paren, luego esperamos.
    for (auto& lector : lectores) {
        lector->stop();
    }
    for (auto& lector : lectores) {
        lector->join();
    }

    logger().info("Sistema detenido correctamente");
    return 0;
}
