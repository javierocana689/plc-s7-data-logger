from datetime import datetime
import os
import config as config

# Paso 2 — Nueva función guardar_vuelta que recibe el diccionario de valores
def guardar_vuelta(ip, valores):
    # Paso 5 — Mantener igual la gestión de fechas y carpetas
    ahora = datetime.now()
    fecha_hoy = ahora.strftime("%Y-%m-%d")

    # Construir la ruta de la carpeta
    carpeta_diaria = os.path.join("logs", fecha_hoy)

    # Crear la carpeta si no existe
    os.makedirs(carpeta_diaria, exist_ok=True)
    
    # El fichero se sigue llamando por la IP
    ruta_fichero = os.path.join(carpeta_diaria, f"{ip}.csv")

    # Comprobar si el fichero existe antes de abrirlo
    fichero_nuevo = not os.path.exists(ruta_fichero)

    # Preparar el timestamp
    timestamp_completo = ahora.strftime("%Y-%m-%d %H:%M:%S")
    
    # Paso 4 — Construir la línea de datos dinámicamente con los valores
    valores_str = ",".join(str(v) for v in valores.values())
    linea = f"{timestamp_completo},{valores_str}\n"

    with open(ruta_fichero, "a", encoding="utf-8") as fichero:
        # Paso 3 — Construir y escribir la cabecera dinámicamente si el fichero es nuevo
        if fichero_nuevo:
            cabecera = "timestamp," + ",".join(valores.keys()) + "\n"
            fichero.write(cabecera)
            
        # Escribir la línea de datos
        fichero.write(linea)