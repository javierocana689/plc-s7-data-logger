// Logger.h — Sistema de logging sencillo y seguro entre hilos.
//
// Equivale al módulo logging de Python (consola + fichero rotativo).
// Varios hilos escriben a la vez, así que todo se protege con un mutex.
#pragma once

#include <string>

enum class Nivel { DEBUG, INFO, ERROR, CRITICAL };

class Logger {
public:
    // Devuelve la única instancia del logger (patrón singleton).
    static Logger& instancia();

    // Configura el fichero de salida y el nivel mínimo que se muestra por consola.
    void configurar(const std::string& rutaFichero, Nivel nivelConsola = Nivel::INFO);

    void log(Nivel nivel, const std::string& mensaje);

    // Atajos para no escribir el nivel cada vez.
    void debug(const std::string& m)    { log(Nivel::DEBUG, m); }
    void info(const std::string& m)     { log(Nivel::INFO, m); }
    void error(const std::string& m)    { log(Nivel::ERROR, m); }
    void critical(const std::string& m) { log(Nivel::CRITICAL, m); }

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;             // No copiable (es un singleton)
    Logger& operator=(const Logger&) = delete;

    void rotarSiHaceFalta();  // Rota el fichero si supera el tamaño máximo

    std::string rutaFichero_;
    Nivel nivelConsola_ = Nivel::INFO;
    long maxBytes_ = 1'000'000;  // 1 MB, como el RotatingFileHandler de Python
    int backupCount_ = 3;        // Conserva los 3 ficheros más recientes
};

// Atajo global para no escribir Logger::instancia() cada vez.
inline Logger& logger() { return Logger::instancia(); }
