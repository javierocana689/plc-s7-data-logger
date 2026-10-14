// main.cpp — Punto de entrada (versión 2_security).
//
// Contraseña del PLC: ver config.json (campo "password")
//
// Añade un primer intento de enviar la contraseña de Full access tras
// conectar. Igual que en tu main.py de esta version: el intento no es
// realmente funcional (ConnectTo ya se hizo sin credenciales; llamar a un
// metodo de "parametros de conexion" después de conectado no cambia nada).
// La forma correcta (SetSessionPassword) llega en la version final (src3).
#include <atomic>
#include <csignal>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

#include <snap7.h>

#include "Config.h"
#include "ConfigValidator.h"
#include "Logger.h"
#include "PlcReader.h"

namespace {
std::atomic<bool> g_salir{false};
void manejadorSenal(int) { g_salir.store(true); }

struct Conexion {
    std::unique_ptr<PlcReader> lector;
    TS7Client* cliente;
};
}  // namespace

int main(int argc, char** argv) {
    const std::string rutaConfig = (argc > 1) ? argv[1] : "config.json";

    logger().configurar("sistema_plc.log", Nivel::INFO);
    logger().info("Iniciando sistema de adquisicion de datos");

    AppConfig config;
    try {
        config = cargarConfig(rutaConfig);
        validarConfig(config);
    } catch (const std::exception& e) {
        logger().critical(std::string("Error en configuracion: ") + e.what());
        return 1;
    }

    std::signal(SIGINT, manejadorSenal);
    std::signal(SIGTERM, manejadorSenal);

    std::vector<Conexion> conexiones;

    for (const auto& plc : config.plcs) {
        TS7Client* cliente = nullptr;

        if (!config.modo_simulacion) {
            cliente = new TS7Client();
            const int resultado = cliente->ConnectTo(plc.ip.c_str(), plc.rack, plc.slot);
            if (resultado != 0) {
                logger().error("No se pudo conectar a " + plc.ip);
                delete cliente;
                continue;
            }

            // Intento (no funcional) de enviar la contraseña tras conectar.
            // SetConnectionType() es una funcion real de snap7, pero sirve
            // para indicar el TIPO de conexion (PG/OP/S7Basic), no para
            // autenticar con contraseña. Es el equivalente exacto al error
            // de tu version Python (llamar a set_connection_params, que
            // tampoco envia la contraseña a ningun sitio).
            if (!plc.password.empty()) {
                cliente->SetConnectionType(static_cast<std::uint16_t>(S7_OP));
            }

            logger().info("Conectado a PLC " + plc.ip);
        } else {
            logger().info("[" + plc.ip + "] Modo simulacion: no se conecta a ningun PLC");
        }

        auto lector = std::make_unique<PlcReader>(cliente, plc.senales, plc.ip, config.intervalo);
        lector->start();

        conexiones.push_back({std::move(lector), cliente});
    }

    if (conexiones.empty()) {
        logger().critical("No se pudo conectar a ningun PLC. Saliendo.");
        return 1;
    }

    while (!g_salir.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    logger().info("Interrupcion recibida. Cerrando sistema...");

    for (auto& c : conexiones) c.lector->stop();
    for (auto& c : conexiones) c.lector->join();
    for (auto& c : conexiones) {
        if (c.cliente) {
            c.cliente->Disconnect();
            logger().info("Desconectado del PLC");
            delete c.cliente;
        }
    }

    return 0;
}
