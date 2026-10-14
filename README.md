# PLC S7 Data Logger

Sistema de adquisición de datos para PLCs **Siemens S7-1200**, que lee señales
por el protocolo **S7comm** (TCP/IP, puerto 102) y las guarda en ficheros CSV
diarios, uno por PLC.

Este repositorio contiene **dos implementaciones** del mismo sistema, en dos
lenguajes distintos:

- **[`python/`](python/README.md)** — Versión original, usando
  [`python-snap7`](https://python-snap7.readthedocs.io/). Incluye además la
  evolución del proyecto a lo largo de varias iteraciones (`src1` → `src3`).
- **[`cpp/`](cpp/README.md)** — Reescritura en **C++17**, usando la librería
  nativa [snap7](https://snap7.sourceforge.net/) directamente, con hilos
  (`std::thread`), configuración en JSON (`nlohmann/json`) y gestión manual
  de memoria y recursos.

Ambas versiones implementan la misma arquitectura: un hilo autónomo por PLC,
con reconexión automática, agrupación de señales por bloque de datos (DB) para
leer en un solo bloque, y escritura a CSV diario por IP.

## ¿Por qué dos lenguajes?

El proyecto nació en Python por rapidez de desarrollo y porque `snap7` tiene
un binding directo (`python-snap7`). La versión en C++ es un ejercicio
deliberado de portar el mismo sistema a un lenguaje compilado y de más bajo
nivel, hablando con la librería nativa de snap7 sin intermediarios: gestión
manual de hilos, conversión manual de bytes a `float` (sin la ayuda de
`snap7.util`), y build con CMake.

## ⚠️ Nota de seguridad

Repositorio **público**: ningún `config.py` / `config.json` con datos reales
está versionado. Las IPs de ejemplo están enmascaradas y las contraseñas son
marcadores (`CAMBIAR_POR_TU_PASSWORD`). Cada subcarpeta explica cómo rellenar
tu propia configuración local sin exponerla.

## Estructura

```
.
├── python/     → Implementación original en Python (ver su README)
└── cpp/        → Reescritura en C++17 (ver su README)
```

Cada carpeta es independiente: tiene su propio README con instrucciones de
instalación, configuración y ejecución.
