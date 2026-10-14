// PlcReader.h — Hilo de lectura para un PLC (versión 1).
//
// Equivalente a tu plc_reader.py original: el cliente ya viene conectado
// desde main() y se pasa por puntero. Se lee señal a señal (sin optimizar;
// esa mejora llega en src2). Usamos std::thread por composición en vez de
// heredar, que es el equivalente idiomático en C++ a heredar de
// threading.Thread en Python.
#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "Config.h"

class TS7Client;

class PlcReader {
public:
    // cliente debe venir ya conectado (lo conecta main antes de crear esto).
    PlcReader(TS7Client* cliente, const std::vector<Senal>& senales,
              const std::string& ip, double intervalo);

    PlcReader(const PlcReader&) = delete;
    PlcReader& operator=(const PlcReader&) = delete;

    void start();  // Arranca el hilo (equivale a Thread.start())
    void stop();   // Señaliza parada (equivale a Event.set())
    void join();   // Espera a que el hilo termine

private:
    void run();

    TS7Client* cliente_;
    std::vector<Senal> senales_;
    std::string ip_;
    double intervalo_;
    std::thread hilo_;
    std::atomic<bool> parar_{false};
};
