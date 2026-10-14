// PlcReader.cpp — Implementación del hilo de lectura por PLC.
#include "PlcReader.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <map>
#include <thread>

#include <snap7.h>

#include "FileManager.h"
#include "Logger.h"

namespace {

// Convierte 4 bytes en formato S7 REAL (float IEEE-754 big-endian) a un float
// nativo. Equivale a snap7.util.get_real() de python-snap7.
//
// El PLC envía el byte más significativo primero (big-endian). Reconstruimos
// el entero de 32 bits en ese orden y luego lo reinterpretamos como float.
// (Asume host little-endian, que es el caso en Windows/Linux sobre x86 y ARM.)
float getRealAt(const std::uint8_t* buffer, int offset) {
    const std::uint32_t crudo =
        (static_cast<std::uint32_t>(buffer[offset])     << 24) |
        (static_cast<std::uint32_t>(buffer[offset + 1]) << 16) |
        (static_cast<std::uint32_t>(buffer[offset + 2]) << 8)  |
        (static_cast<std::uint32_t>(buffer[offset + 3]));

    float valor;
    std::memcpy(&valor, &crudo, sizeof(valor));  // Bit-cast seguro (sin UB)
    return valor;
}

// Duerme en tramos cortos para poder reaccionar rápido a una petición de parada.
void dormirReactivo(double segundos, const std::atomic<bool>& parar) {
    const auto fin = std::chrono::steady_clock::now() +
                     std::chrono::duration<double>(segundos);
    while (!parar.load() && std::chrono::steady_clock::now() < fin) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

}  // namespace

PlcReader::PlcReader(const PlcConfig& plc, double intervalo,
                     double intervaloReconexion, bool modoSimulacion)
    : plc_(plc),
      intervalo_(intervalo),
      intervaloReconexion_(intervaloReconexion),
      modoSimulacion_(modoSimulacion),
      gruposDb_(agruparPorDb(plc.senales)) {}

PlcReader::~PlcReader() {
    if (hilo_.joinable()) {
        stop();
        hilo_.join();
    }
    delete cliente_;
}

std::vector<PlcReader::GrupoDB>
PlcReader::agruparPorDb(const std::vector<Senal>& senales) const {
    // Agrupamos las señales por número de DB. Usamos un std::map para que los
    // grupos queden ordenados por DB (equivale a sorted + groupby en Python).
    std::map<int, std::vector<Senal>> porDb;
    for (const auto& senal : senales) {
        porDb[senal.db].push_back(senal);
    }

    std::vector<GrupoDB> grupos;
    for (const auto& [db, senalesDelGrupo] : porDb) {
        GrupoDB grupo;
        grupo.db = db;
        grupo.senales = senalesDelGrupo;

        // offset inicial = el menor offset; offset final = el mayor (offset+bytes)
        int offsetIni = senalesDelGrupo.front().offset;
        int offsetFin = 0;
        for (const auto& s : senalesDelGrupo) {
            offsetIni = std::min(offsetIni, s.offset);
            offsetFin = std::max(offsetFin, s.offset + s.bytes);
        }
        grupo.offsetInicial = offsetIni;
        grupo.totalBytes = offsetFin - offsetIni;

        grupos.push_back(grupo);
    }
    return grupos;
}

void PlcReader::start() {
    hilo_ = std::thread(&PlcReader::run, this);
}

void PlcReader::stop() {
    parar_.store(true);
}

void PlcReader::join() {
    if (hilo_.joinable()) {
        hilo_.join();
    }
}

bool PlcReader::conectar() {
    if (modoSimulacion_) {
        logger().info("[" + plc_.ip + "] Modo simulacion: no se conecta a ningun PLC");
        return true;
    }

    while (!parar_.load()) {
        logger().info("[" + plc_.ip + "] Intentando conectar...");

        const int resultado = cliente_->ConnectTo(plc_.ip.c_str(), plc_.rack, plc_.slot);
        if (resultado == 0) {
            // Si hay contraseña configurada, la enviamos. Esta es la forma
            // correcta en snap7 (SetSessionPassword), a diferencia del intento
            // con set_connection_params que no funcionaba en la versión Python.
            if (!plc_.password.empty()) {
                std::vector<char> pass(plc_.password.begin(), plc_.password.end());
                pass.push_back('\0');
                cliente_->SetSessionPassword(pass.data());
            }
            logger().info("[" + plc_.ip + "] Conectado correctamente");
            return true;
        }

        logger().error("[" + plc_.ip + "] Conexion fallida (codigo 0x" +
                       [resultado] {
                           char buf[16];
                           std::snprintf(buf, sizeof(buf), "%08X", resultado);
                           return std::string(buf);
                       }() + ")");
        dormirReactivo(intervaloReconexion_, parar_);
    }
    return false;
}

void PlcReader::run() {
    logger().info("[" + plc_.ip + "] Hilo iniciado");

    if (!modoSimulacion_) {
        cliente_ = new TS7Client();
    }

    // Conexión inicial — si falla, reintenta automáticamente dentro de conectar().
    if (!conectar()) {
        logger().info("[" + plc_.ip + "] Hilo detenido antes de conectar");
        return;
    }

    // Semilla simple para generar valores en modo simulación.
    float sim = 0.0f;

    while (!parar_.load()) {
        Lecturas valores;

        for (const auto& grupo : gruposDb_) {
            if (modoSimulacion_) {
                // Generamos valores sintéticos para poder probar sin PLC real.
                for (const auto& senal : grupo.senales) {
                    valores.emplace_back(senal.nombre, 20.0f + sim);
                    sim += 0.1f;
                }
                continue;
            }

            // 1. Una sola lectura al PLC para todo el bloque del DB.
            std::vector<std::uint8_t> datos(grupo.totalBytes);
            const int resultado = cliente_->DBRead(
                grupo.db, grupo.offsetInicial, grupo.totalBytes, datos.data());

            if (resultado != 0) {
                logger().error("[" + plc_.ip + "] Error leyendo DB" +
                               std::to_string(grupo.db) + "; intentando reconectar");
                if (!conectar()) break;  // Si no reconecta (parada), salimos
                continue;
            }

            // 2. Extraemos cada señal de su posición dentro del bloque.
            //    Ojo: el offset de la señal es absoluto dentro del DB, así que
            //    restamos el offset inicial del bloque que hemos leído.
            for (const auto& senal : grupo.senales) {
                const int pos = senal.offset - grupo.offsetInicial;
                const float valor = getRealAt(datos.data(), pos);
                logger().debug("[" + plc_.ip + "] " + senal.nombre + ": " +
                               std::to_string(valor));
                valores.emplace_back(senal.nombre, valor);
            }
        }

        // 3. Guardamos todas las señales de esta vuelta en el CSV.
        if (!valores.empty()) {
            guardarVuelta(plc_.ip, valores);
        }

        // 4. Esperamos el intervalo de polling (reaccionando a la parada).
        dormirReactivo(intervalo_, parar_);
    }

    // Desconexión limpia al salir.
    if (cliente_ && cliente_->Connected()) {
        cliente_->Disconnect();
        logger().info("[" + plc_.ip + "] Desconectado del PLC");
    }
    logger().info("[" + plc_.ip + "] Hilo detenido");
}
