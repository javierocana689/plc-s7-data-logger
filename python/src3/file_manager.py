"""Gestión de ficheros CSV diarios para almacenamiento de lecturas."""

from datetime import datetime
import os
import logging

logger = logging.getLogger(__name__)


def guardar_vuelta(ip, valores):
    """
    Guarda una vuelta de lecturas en el CSV diario del PLC.

    Args:
        ip (str): IP del PLC, usada como nombre de fichero.
        valores (dict): Diccionario {nombre_señal: valor_float}.
    """
    ahora = datetime.now()
    fecha_hoy = ahora.strftime("%Y-%m-%d")
    carpeta_diaria = os.path.join("logs", fecha_hoy)
    os.makedirs(carpeta_diaria, exist_ok=True)

    ruta_fichero = os.path.join(carpeta_diaria, f"{ip}.csv")
    fichero_nuevo = not os.path.exists(ruta_fichero)
    timestamp_completo = ahora.strftime("%Y-%m-%d %H:%M:%S")

    valores_str = ",".join(str(v) for v in valores.values())
    linea = f"{timestamp_completo},{valores_str}\n"

    with open(ruta_fichero, "a", encoding="utf-8-sig") as fichero:  # utf-8-sig para Excel
        if fichero_nuevo:
            cabecera = "timestamp," + ",".join(valores.keys()) + "\n"
            fichero.write(cabecera)
            logger.info(f"Nuevo fichero CSV creado: {ruta_fichero}")
        fichero.write(linea)