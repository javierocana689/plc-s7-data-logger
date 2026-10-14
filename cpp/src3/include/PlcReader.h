// PlcReader.h — Hilo de lectura autónomo para un PLC Siemens S7-1200.
//
// Equivale a tu plc_reader.py (versión src3): cada instancia gestiona un PLC
// de principio a fin —conexión, reconexión automática, polling y desconexión—
// en su propio hilo. Hereda la idea de threading.Thread usando std::thread,
// y el "interruptor" de threading.Event se implementa con std::atomic<bool>.
#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "Config.h"

// Declaración adelantada para no incluir snap7.h en la cabecera.
class TS7Client;

class PlcReader {
public:
    PlcReader(const PlcConfig& plc, double intervalo, double intervaloReconexion,
              bool modoSimulacion);
    ~PlcReader();

    // No copiable ni movible: contiene un hilo y un puntero a recurso nativo.
    PlcReader(const PlcReader&) = delete;
    PlcReader& operator=(const PlcReader&) = delete;

    void start();  // Arranca el hilo (equivale a Thread.start())
    void stop();   // Señaliza al hilo para que termine (equivale a Event.set())
    void join();   // Espera a que el hilo termine (equivale a Thread.join())

private:
    // Un grupo de señales que comparten el mismo DB, con sus offsets ya
    // precalculados para leer todo el bloque de una sola vez.
    struct GrupoDB {
        int db = 0;
        int offsetInicial = 0;
        int totalBytes = 0;
        std::vector<Senal> senales;
    };

    void run();                 // Bucle principal del hilo
    bool conectar();            // Conecta (con reintentos) hasta lograrlo o parar
    std::vector<GrupoDB> agruparPorDb(const std::vector<Senal>& senales) const;

    PlcConfig plc_;
    double intervalo_;
    double intervaloReconexion_;
    bool modoSimulacion_;

    std::vector<GrupoDB> gruposDb_;   // Precalculado una vez en el constructor
    TS7Client* cliente_ = nullptr;    // Cliente nativo de snap7 (se crea en run)
    std::thread hilo_;
    std::atomic<bool> parar_{false};  // El "interruptor" de parada
};
