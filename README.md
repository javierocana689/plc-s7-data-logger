# PLC S7 Data Logger

Sistema de adquisición de datos para PLCs **Siemens S7-1200** escrito en Python.
Lee señales de los PLCs mediante el protocolo **S7comm** (sobre TCP/IP, puerto 102)
usando la librería [`python-snap7`](https://python-snap7.readthedocs.io/) y las
guarda en ficheros CSV diarios, uno por PLC.

> Proyecto desarrollado en un entorno industrial (planta de EMASA) sobre un PC
> Windows aislado de la red. Este repositorio recoge la evolución del código a lo
> largo de varias iteraciones, de la versión más simple a la más completa.

---

## ⚠️ Nota de seguridad

Este repositorio es **público**, así que **no contiene datos reales**. En los
ficheros `config.py` las IPs aparecen enmascaradas (`192.168.0.**`,
`10.240.***.***`) y la contraseña del PLC es un marcador (`CAMBIAR_POR_TU_PASSWORD`).

Antes de ejecutar el programa debes sustituir esos valores por los reales de tu
instalación. **Nunca subas a un repositorio público la IP interna real ni la
contraseña de tu PLC.** Si en algún momento pones valores reales, guárdalos en un
fichero local (por ejemplo `config.local.py`, ya excluido en `.gitignore`).

---

## Arquitectura

El sistema se organiza en cinco módulos:

- **`config.py`** — Lista de PLCs (`ip`, `rack`, `slot`, `password`, `senales`) y
  parámetros globales (intervalo de polling y de reconexión).
- **`config_validator.py`** — Valida que cada PLC y cada señal tengan los campos
  obligatorios antes de arrancar.
- **`file_manager.py`** — Escribe los CSV diarios por PLC (carpeta `logs/AAAA-MM-DD/`),
  con codificación `utf-8-sig` para que Excel muestre bien los acentos.
- **`plc_reader.py`** — Hilo (`threading.Thread`) que gestiona un PLC de forma
  autónoma: conexión, reconexión automática, polling y apagado ordenado.
- **`main.py`** — Punto de entrada: configura el logging, valida la configuración,
  lanza un hilo por PLC y gestiona el cierre con `Ctrl+C`.

## Evolución del código

Cada carpeta es una iteración del proyecto:

| Carpeta          | Qué introduce |
|------------------|---------------|
| `src1`           | Primera versión funcional: hilo por PLC, lectura señal a señal, CSV por IP. |
| `src1_2`         | Añade `logging` (consola + fichero rotativo) y valida la configuración. |
| `src2`           | Optimiza la lectura: una sola lectura de bloque por PLC en lugar de N lecturas. |
| `src2_security`  | Explora el paso de contraseña de *Full access* del PLC. |
| `src3`           | Versión más avanzada: `PLCReader` totalmente autónomo (crea y gestiona su propio cliente), reconexión automática y agrupación de señales por DB con `itertools.groupby`. |

La versión de referencia es **`src3`**.

## Requisitos

- Python 3.x (Windows / Linux)
- `python-snap7==3.0.0` (requiere la librería nativa `snap7.dll` / `libsnap7`)
- Un PLC Siemens S7-1200 accesible por red, con el nivel de acceso de lectura
  activado en TIA Portal.

Instalación de dependencias:

```bash
pip install -r requirements.txt
```

## Uso

1. Edita `config.py` (dentro de la carpeta de la versión que quieras usar, p. ej.
   `src3/`) y pon la IP, rack, slot y señales reales de tu PLC.
2. Ejecuta el punto de entrada desde esa carpeta:

```bash
cd src3
python main.py
```

3. Los datos se guardan en `logs/AAAA-MM-DD/<ip>.csv`. Detén el programa con `Ctrl+C`.

## Próximos pasos

- Soporte para más tipos de dato además de `REAL` (enteros, booleanos)
- Alertas por umbral
- Dashboard de consola
- Modo simulación
- Watchdog
- Descubrimiento de señales por OPC-UA (`asyncua`) para no configurar `config.py` a mano
