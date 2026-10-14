"""Validación de la configuración antes de arrancar el sistema."""

import logging

logger = logging.getLogger(__name__)

# Claves obligatorias en cada PLC
CLAVES_PLC = {'ip', 'rack', 'slot', 'senales'}

# Claves obligatorias en cada señal
CLAVES_SENAL = {'nombre', 'db', 'offset', 'bytes'}


def validar_config(plcs):
    """
    Valida la lista de PLCs del fichero de configuración.

    Args:
        plcs (list): Lista de diccionarios de PLCs desde config.py.

    Returns:
        bool: True si la configuración es válida, False si hay errores.
    """
    print(f"Validando {len(plcs)} PLCs")
    for i, plc in enumerate(plcs):
        
        claves_faltantes = CLAVES_PLC - plc.keys()
        
        if claves_faltantes:
            logger.critical(f"PLC #{i} le faltan los campos: {claves_faltantes}")
            return False


        for j, senal in enumerate(plc['senales']):
            claves_faltantes = CLAVES_SENAL - senal.keys()
            if claves_faltantes:
                logger.critical(
                    f"PLC #{i} señal #{j} le faltan los campos: {claves_faltantes}"
                )
                return False

    return True