// Logger.cpp — Implementación del logger seguro entre hilos.
#include "Logger.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>

namespace {

std::mutex g_mutex;  // Protege la escritura concurrente desde varios hilos

// Devuelve la fecha/hora actual formateada (multiplataforma: Windows y Linux).
std::string ahoraFormateada() {
    const auto ahora = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(ahora);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return buffer;
}

const char* textoNivel(Nivel nivel) {
    switch (nivel) {
        case Nivel::DEBUG:    return "DEBUG";
        case Nivel::INFO:     return "INFO";
        case Nivel::ERROR:    return "ERROR";
        case Nivel::CRITICAL: return "CRITICAL";
    }
    return "?";
}

}  // namespace

Logger& Logger::instancia() {
    static Logger instancia;  // Inicialización perezosa y segura entre hilos (C++11)
    return instancia;
}

Logger::~Logger() = default;

void Logger::configurar(const std::string& rutaFichero, Nivel nivelConsola) {
    std::lock_guard<std::mutex> lock(g_mutex);
    rutaFichero_ = rutaFichero;
    nivelConsola_ = nivelConsola;
}

void Logger::rotarSiHaceFalta() {
    if (rutaFichero_.empty()) return;

    std::ifstream f(rutaFichero_, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return;

    const long tam = static_cast<long>(f.tellg());
    f.close();
    if (tam < maxBytes_) return;

    // Desplaza los backups: .2 -> .3, .1 -> .2, fichero -> .1
    std::remove((rutaFichero_ + "." + std::to_string(backupCount_)).c_str());
    for (int i = backupCount_ - 1; i >= 1; --i) {
        std::rename((rutaFichero_ + "." + std::to_string(i)).c_str(),
                    (rutaFichero_ + "." + std::to_string(i + 1)).c_str());
    }
    std::rename(rutaFichero_.c_str(), (rutaFichero_ + ".1").c_str());
}

void Logger::log(Nivel nivel, const std::string& mensaje) {
    std::lock_guard<std::mutex> lock(g_mutex);

    const std::string linea =
        ahoraFormateada() + " [" + textoNivel(nivel) + "] " + mensaje;

    // Consola: solo si el nivel llega al umbral configurado.
    if (nivel >= nivelConsola_) {
        if (nivel == Nivel::ERROR || nivel == Nivel::CRITICAL) {
            std::cerr << linea << std::endl;
        } else {
            std::cout << linea << std::endl;
        }
    }

    // Fichero: siempre se escribe todo (como el handler de fichero de Python).
    if (!rutaFichero_.empty()) {
        rotarSiHaceFalta();
        std::ofstream f(rutaFichero_, std::ios::app);
        if (f.is_open()) {
            f << linea << '\n';
        }
    }
}
