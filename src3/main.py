"""Punto de entrada del sistema de adquisición de datos PLC."""

import time
import logging
from logging.handlers import RotatingFileHandler
import config
import plc_reader
from config_validator import validar_config


def configurar_logging():
    """
    Configura el sistema de logging con salida a consola y fichero rotativo.

    El fichero no supera 1MB. Cuando se llena, crea uno nuevo y
    conserva los 3 más recientes.
    """
    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    formato = logging.Formatter(
        "%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S"
    )

    consola = logging.StreamHandler()
    consola.setLevel(logging.INFO)
    consola.setFormatter(formato)

    fichero = RotatingFileHandler(
        "sistema_plc.log", maxBytes=1_000_000, backupCount=3, encoding="utf-8"
    )
    fichero.setLevel(logging.DEBUG)
    fichero.setFormatter(formato)

    logger.addHandler(consola)
    logger.addHandler(fichero)


def main():
    configurar_logging()
    logger = logging.getLogger(__name__)
    logger.info("Iniciando sistema de adquisición de datos")

    # Validar configuración antes de arrancar
    if not validar_config(config.PLCs):
        logger.critical("Error en configuración. Saliendo.")
        return

    # Una sola lista — cada elemento es un lector autónomo
    conexiones = []

    for plc in config.PLCs:
        lector = plc_reader.PLCReader(
            senales=plc['senales'],
            ip=plc['ip'],
            rack=plc['rack'],
            slot=plc['slot']
        )
        lector.start()
        conexiones.append({'lector': lector})

    if not conexiones:
        logger.critical("No se pudo crear ningún lector. Saliendo.")
        return

    # Mantener el programa vivo hasta Ctrl+C
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        logger.info("Interrupción recibida. Cerrando sistema...")

    # Apagado ordenado
    for conexion in conexiones:
        conexion['lector'].stop()
    for conexion in conexiones:
        conexion['lector'].join()

    logger.info("Sistema detenido correctamente")


if __name__ == "__main__":
    main()