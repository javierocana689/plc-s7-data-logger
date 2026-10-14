// PlcReader.h — Hilo de lectura para un PLC (versión 2: lectura optimizada).
//
// Equivalente a tu plc_reader.py de esta versión: se calcula una sola vez
// (antes del bucle) el offset_inicial, offset_final y total_bytes de todas
// las señales, y se hace UNA lectura de bloque por vuelta en vez de N.
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
