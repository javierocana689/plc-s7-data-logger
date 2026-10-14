// FileManager.cpp — Implementación del guardado en CSV diario (sin logging aún).
#include "FileManager.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

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
        std::cerr << "No se pudo crear la carpeta " << carpetaDiaria << ": " << ec.message() << '\n';
        return;
    }

    const fs::path rutaFichero = carpetaDiaria / (ip + ".csv");
    const bool ficheroNuevo = !fs::exists(rutaFichero);

    std::ofstream fichero(rutaFichero, std::ios::app);
    if (!fichero.is_open()) {
        std::cerr << "No se pudo abrir el CSV " << rutaFichero << '\n';
        return;
    }

    if (ficheroNuevo) {
        std::ostringstream cabecera;
        cabecera << "timestamp";
        for (const auto& [nombre, _] : valores) cabecera << "," << nombre;
        fichero << cabecera.str() << '\n';
    }

    std::ostringstream linea;
    linea << timestamp;
    for (const auto& [_, valor] : valores) linea << "," << valor;
    fichero << linea.str() << '\n';
}
