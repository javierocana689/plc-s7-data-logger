# PLC S7 Data Logger (C++)

Sistema de adquisición de datos para PLCs **Siemens S7-1200** escrito en **C++17**.
Lee señales de los PLCs mediante el protocolo **S7comm** (sobre TCP/IP, puerto 102)
usando la librería nativa [**snap7**](https://snap7.sourceforge.net/) y las guarda
en ficheros CSV diarios, uno por PLC.

Es la versión en C++ de [mi logger original en Python](../python/README.md), reescrita para demostrar
el mismo sistema en un lenguaje compilado y de más bajo nivel. La arquitectura es
equivalente a la versión más avanzada de aquel proyecto: cada PLC corre en su propio
hilo autónomo, con reconexión automática y lectura de bloque optimizada.

---

## ⚠️ Nota de seguridad

Repositorio **público**: no contiene datos reales. La configuración de ejemplo
(`config.example.json`) lleva la IP enmascarada (`192.168.0.**`) y la contraseña
como marcador (`CAMBIAR_POR_TU_PASSWORD`). Tu `config.json` real —con la IP y la
contraseña verdaderas— está en el `.gitignore` y **nunca** debe subirse.

---

## Arquitectura

| Módulo | Responsabilidad |
|---|---|
| `Config.h/.cpp`       | Define las estructuras (`PlcConfig`, `Senal`…) y carga/valida el `config.json`. |
| `Logger.h/.cpp`       | Logging seguro entre hilos a consola y a fichero rotativo (1 MB × 3). |
| `FileManager.h/.cpp`  | Escribe los CSV diarios por PLC en `logs/AAAA-MM-DD/<ip>.csv`. |
| `PlcReader.h/.cpp`    | Hilo autónomo por PLC: conexión, reconexión, polling y desconexión. |
| `main.cpp`            | Arranca un hilo por PLC y gestiona el apagado ordenado con `Ctrl+C`. |

### De Python a C++

| En la versión Python | Aquí (C++) |
|---|---|
| `python-snap7` | snap7 nativo (`TS7Client`) |
| `config.py` (lista de dicts) | `config.json` + `nlohmann/json` |
| `threading.Thread` + `threading.Event` | `std::thread` + `std::atomic<bool>` |
| `snap7.util.get_real()` | `getRealAt()` (byteswap big-endian → `float`) |
| módulo `logging` | clase `Logger` (singleton + `std::mutex`) |
| `itertools.groupby` por DB | agrupación con `std::map<int, ...>` |

> Detalle: el paso de la contraseña usa `SetSessionPassword()`, que es la forma
> correcta en snap7 (en la versión Python se intentó con `set_connection_params`,
> que no era funcional).

## Dependencias

- Un compilador con **C++17** (MSVC, g++ o clang) y **CMake ≥ 3.15**.
- **nlohmann/json** — se descarga sola al compilar (vía `FetchContent`).
- **snap7** (nativo) — la pones tú en `third_party/snap7/`. Ver
  [`third_party/snap7/README.md`](third_party/snap7/README.md).

## Compilación

```bash
cmake -S . -B build
cmake --build build
```

El ejecutable queda en `build/` (p. ej. `build/plc_logger` o `build/Debug/plc_logger.exe`).
En Windows, la `snap7.dll` se copia automáticamente junto al `.exe`.

## Uso

1. Copia `config.example.json` a `config.json` y pon los datos reales de tu PLC.
2. Ejecuta (la ruta del config es opcional; por defecto `config.json`):

```bash
./plc_logger            # usa ./config.json
./plc_logger otra.json  # usa otra ruta
```

3. Los datos se guardan en `logs/AAAA-MM-DD/<ip>.csv`. Para en cualquier momento con `Ctrl+C`.

## Modo simulación (probar sin PLC)

Para ver el sistema funcionando sin un PLC físico, pon `"modo_simulacion": true`
en el `config.json`. En ese modo no se conecta a ningún equipo: genera valores
sintéticos y los escribe en el CSV, útil para probar el logging, los hilos y la
generación de ficheros. (Nota: sigue necesitando snap7 para **compilar**, solo
que no lo usa para leer.)

## Posibles mejoras

- Soporte para más tipos de dato además de `REAL` (enteros, booleanos).
- Alertas por umbral.
- Lectura por OPC-UA como alternativa a S7comm.
