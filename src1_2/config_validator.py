"""Validación de la configuración antes de arrancar el sistema."""

# Claves obligatorias en cada PLC
CLAVES_PLC = {'ip', 'rack', 'slot', 'senales'}

# Claves obligatorias en cada señal
CLAVES_SENAL = {'nombre', 'db', 'offset', 'bytes'}


def validar_config(plcs):
    """
    Valida la lista de PLCs del fichero de configuración.

    Args:
        plcs (list): Lista de diccionarios de PLCs desde config.py.

    Raises:
        ValueError: Si falta algún campo obligatorio.
    """
    for i, plc in enumerate(plcs):
        # Comprobamos las claves del PLC
        claves_faltantes = CLAVES_PLC - plc.keys()
        if claves_faltantes:
            raise ValueError(f"PLC #{i} le faltan los campos: {claves_faltantes}")

        # Comprobamos las claves de cada señal
        for j, senal in enumerate(plc['senales']):
            claves_faltantes = CLAVES_SENAL - senal.keys()
            if claves_faltantes:
                raise ValueError(
                    f"PLC #{i} señal #{j} le faltan los campos: {claves_faltantes}"
                )