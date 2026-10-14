"""Punto de entrada del sistema de adquisición de datos PLC."""

import time
import logging
from logging.handlers import RotatingFileHandler
import snap7
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


def ad()->bool:
    status = True
    try:
        validar_config(config.PLCs)
    except ValueError as e:
        logger.critical(f"Error en configuración: {e}")
        status = False
    return status

def main():
    configurar_logging()
    logger = logging.getLogger(__name__)
    logger.info("Iniciando sistema de adquisición de datos")

    try:
        validar_config(config.PLCs)
    except ValueError as e:
        logger.critical(f"Error en configuración: {e}")
        return

    # Una sola lista de diccionarios — cada elemento agrupa lector y cliente del mismo PLC
    conexiones = []

    for plc in config.PLCs:
        try:
            cliente = snap7.client.Client()
            cliente.connect(plc['ip'], plc['rack'], plc['slot'])
            logger.info(f"Conectado a PLC {plc['ip']}")
        except Exception as e:
            logger.error(f"No se pudo conectar a {plc['ip']}: {e}")
            continue

        lector = plc_reader.PLCReader(cliente, plc['senales'], plc['ip'])
        lector.start()

        # Lector y cliente juntos en un diccionario
        conexiones.append({'lector': lector, 'cliente': cliente})

    if not conexiones:
        logger.critical("No se pudo conectar a ningún PLC. Saliendo.")
        return

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        logger.info("Interrupción recibida. Cerrando sistema...")

    # Apagado ordenado iterando sobre la misma lista
    for conexion in conexiones:
        conexion['lector'].stop()
    for conexion in conexiones:
        conexion['lector'].join()
    for conexion in conexiones:
        conexion['cliente'].disconnect()
        logger.info(f"Desconectado del PLC {conexion['cliente']}")


if __name__ == "__main__":
    main()