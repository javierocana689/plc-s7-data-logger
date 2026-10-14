// FileManager.cpp — Guardado en CSV diario, con logging y BOM UTF-8.
#include "FileManager.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Logger.h"

namespace fs = std::filesystem;

namespace {

std::pair<std::string, std::string> fechaYHora() {
    const auto ahora = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(ahora);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    char fecha[16];
    char completo[32];
    std::strftime(fecha, sizeof(fecha), "%Y-%m-%d", &tm);
    std::strftime(completo, sizeof(completo), "%Y-%m-%d %H:%M:%S", &tm);
    return {fecha, completo};
}

}  // namespace

void guardarVuelta(const std::string& ip, const Lecturas& valores) {
    const auto [fechaHoy, timestamp] = fechaYHora();

    const fs::path carpetaDiaria = fs::path("logs") / fechaHoy;
    std::error_code ec;
    fs::create_directories(carpetaDiaria, ec);
    if (ec) {
        logger().error("No se pudo crear la carpeta " + carpetaDiaria.string() + ": " + ec.message());
        return;
    }

    const fs::path rutaFichero = carpetaDiaria / (ip + ".csv");
    const bool ficheroNuevo = !fs::exists(rutaFichero);

    std::ofstream fichero(rutaFichero, std::ios::app | std::ios::binary);
    if (!fichero.is_open()) {
        logger().error("No se pudo abrir el CSV " + rutaFichero.string());
        return;
    }

    if (ficheroNuevo) {
        // BOM UTF-8 (EF BB BF), equivalente a la codificacion utf-8-sig de Python.
        const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
        fichero.write(reinterpret_cast<const char*>(bom), sizeof(bom));

        std::ostringstream cabecera;
        cabecera << "timestamp";
        for (const auto& [nombre, _] : valores) cabecera << "," << nombre;
        fichero << cabecera.str() << '\n';
        logger().info("Nuevo fichero CSV creado: " + rutaFichero.string());
    }

    std::ostringstream linea;
    linea << timestamp;
    for (const auto& [_, valor] : valores) linea << "," << valor;
    fichero << linea.str() << '\n';
}
