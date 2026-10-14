// PlcReader.h — Hilo de lectura para un PLC (versión 1.2).
//
// Misma idea que src1: cliente ya conectado, lectura señal a señal. El
// único cambio real aquí es que ahora usa el Logger en vez de std::cout.
#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "Config.h"

class TS7Client;

class PlcReader {
public:
    PlcReader(TS7Client* cliente, const std::vector<Senal>& senales,
              const std::string& ip, double intervalo);

    PlcReader(const PlcReader&) = delete;
    PlcReader& operator=(const PlcReader&) = delete;

    void start();
    void stop();
    void join();

private:
    void run();

    TS7Client* cliente_;
    std::vector<Senal> senales_;
    std::string ip_;
    double intervalo_;
    std::thread hilo_;
    std::atomic<bool> parar_{false};
};
