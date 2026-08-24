"""Hilo de lectura continua para un PLC Siemens S7 mediante S7comm."""

import threading
import time
import logging
import snap7
import snap7.util
import config
import file_manager

logger = logging.getLogger(__name__)


class PLCReader(threading.Thread):
    """
    Hilo de lectura continua de un PLC Siemens S7-1200.

    Hereda de threading.Thread. Cada instancia gestiona la conexión
    y el polling de señales de un PLC concreto.

    Args:
        cliente: Instancia conectada de snap7.client.Client.
        senales (list): Lista de señales a leer (dict con nombre, db, offset, bytes).
        ip (str): IP del PLC, usada para identificar el fichero CSV.
    """

    def __init__(self, cliente, senales, ip):
        super().__init__()
        self.daemon = True          # El hilo muere si muere el programa principal
        self.cliente = cliente
        self.senales = senales
        self.ip = ip
        self._stop_event = threading.Event()

    def run(self):
        """Bucle principal de polling. Se ejecuta hasta que se llame a stop()."""
        logger.info(f"[{self.ip}] Hilo iniciado")

        # Calculado una sola vez — nunca cambia durante la ejecución
        offset_inicial = min(senal['offset'] for senal in self.senales)
        offset_final = max(senal['offset'] + senal['bytes'] for senal in self.senales)
        total_bytes = offset_final - offset_inicial

        #Otra forma, de calcular la cantidad de bytes:
        #total_bytes = sum(senal['bytes'] for senal in self.Senales)
        # En nuestro caso el offset inicial parece que siempre es 0

        while not self._stop_event.is_set():
            valores = {}
            
            try:
                # 1. Una sola lectura al PLC — todos los bytes del bloque de golpe
                datos = self.cliente.db_read(self.senales[0]['db'], offset_inicial, total_bytes)

                for senal in self.senales:
                    # 2. Extraer cada señal de su posición dentro del bloque
                    valor = snap7.util.get_real(datos, senal['offset'])
                    logger.debug(f"[{self.ip}] {senal['nombre']}: {valor}")
                    valores[senal['nombre']] = valor

            except Exception as e:
                logger.error(f"[{self.ip}] Error leyendo bloque: {e}")

            # 3. Guardar todas las señales de esta vuelta en el CSV
            if valores:
                file_manager.guardar_vuelta(self.ip, valores)

            # 4. Esperar el intervalo de polling
            time.sleep(config.Intervalo)

        logger.info(f"[{self.ip}] Hilo detenido")

    def stop(self):
        """Señaliza al hilo para que termine en la próxima iteración."""
        self._stop_event.set()