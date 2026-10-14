# PLC S7 Data Logger (C++)

Sistema de adquisición de datos para PLCs **Siemens S7-1200** escrito en **C++17**.
Lee señales de los PLCs mediante el protocolo **S7comm** (sobre TCP/IP, puerto 102)
usando la librería nativa [**snap7**](https://snap7.sourceforge.net/) y las guarda
en ficheros CSV diarios, uno por PLC.

Es la versión en C++ de [mi logger original en Python](../python/README.md),
reescrita para demostrar el mismo sistema en un lenguaje compilado y de más
bajo nivel, hablando directamente con la librería nativa de snap7.

---

## ⚠️ Nota de seguridad

Repositorio **público**: ningún `config.json` con datos reales está
versionado. Cada `config.example.json` lleva la IP enmascarada
(`192.168.0.**`) y la contraseña como marcador (`CAMBIAR_POR_TU_PASSWORD`).
Tu `config.json` real está en el `.gitignore` y **nunca** debe subirse.

---

## Evolución del código

Igual que en la versión Python, cada carpeta es una iteración del proyecto,
con la **misma progresión** de una versión a la siguiente:

| Carpeta          | Qué introduce (equivalente a su versión en Python) |
|------------------|------------------------------------------------------|
| `src1`           | Primera versión funcional: cliente conectado desde `main`, lectura señal a señal, sin logging (solo `std::cout`), sin validador conectado. |
| `src1_2`         | Añade el módulo `Logger` (consola + fichero rotativo) y `ConfigValidator` (lanza excepción si falta un campo). Fusiona lector+cliente en una sola lista `Conexion`. |
| `src2`           | Optimiza la lectura: una sola llamada a `DBRead` por bloque en vez de una por señal. |
| `src2_security`  | Añade el campo `password` a la configuración y un primer intento (no funcional) de enviarlo tras conectar — igual que el intento fallido con `set_connection_params` en Python. |
| `src3`           | Versión más avanzada: `PlcReader` totalmente autónomo (crea y gestiona su propio `TS7Client`), reconexión automática, señales agrupadas por DB, y el envío de contraseña ahora correcto (`SetSessionPassword`). El validador pasa a devolver `bool` en vez de lanzar excepción. |

La versión de referencia es **`src3`**.

### De Python a C++ (en todas las versiones)

| En la versión Python | Aquí (C++) |
|---|---|
| `python-snap7` | snap7 nativo (`TS7Client`) |
| `config.py` (lista de dicts) | `config.json` + `nlohmann/json` |
| `threading.Thread` + `threading.Event` | `std::thread` + `std::atomic<bool>` |
| `snap7.util.get_real()` | `getRealAt()` (byteswap big-endian → `float`) |
| módulo `logging` | clase `Logger` (singleton + `std::mutex`) |
| `itertools.groupby` por DB (solo en `src3`) | agrupación con `std::map<int, ...>` |

## Dependencias

- Un compilador con **C++17** (MSVC, g++ o clang) y **CMake ≥ 3.15**.
- **nlohmann/json** — se descarga sola al compilar (vía `FetchContent`), en cada subproyecto.
- **snap7** (nativo) — la pones tú **una sola vez** en `third_party/snap7/`
  (se comparte entre las 5 versiones). Ver
  [`third_party/snap7/README.md`](third_party/snap7/README.md).

## Compilación y uso

Cada carpeta (`src1`, `src1_2`, `src2`, `src2_security`, `src3`) es un
proyecto CMake independiente, igual que cada carpeta de `python/` se ejecuta
por separado. Para compilar y ejecutar una versión concreta:

```bash
cd src3                       # o src1, src1_2, src2, src2_security
cmake -S . -B build
cmake --build build
```

El ejecutable queda en `build/` (p. ej. `build/plc_logger_src3` en Linux, o
`build/Debug/plc_logger_src3.exe` en Windows con `snap7.dll` copiada junto a él).

1. Copia `config.example.json` a `config.json` dentro de esa misma carpeta y
   pon los datos reales de tu PLC.
2. Ejecuta (la ruta del config es opcional; por defecto `config.json`):

```bash
./build/plc_logger_src3            # usa ./config.json
./build/plc_logger_src3 otra.json  # usa otra ruta
```

Los datos se guardan en `logs/AAAA-MM-DD/<ip>.csv`. Para en cualquier momento con `Ctrl+C`.

## Modo simulación (probar sin PLC)

Todas las versiones aceptan `"modo_simulacion": true` en el `config.json`:
no se conecta a ningún equipo, genera valores sintéticos y los escribe en el
CSV. Útil para probar el logging, los hilos y la generación de ficheros sin
hardware real. (Sigue necesitando snap7 para **compilar**, solo que no lo usa
para leer.)

## Posibles mejoras

- Soporte para más tipos de dato además de `REAL` (enteros, booleanos).
- Alertas por umbral.
- Lectura por OPC-UA como alternativa a S7comm.
