"""Hilo de lectura continua para un PLC Siemens S7 mediante S7comm."""

import threading
import time
import logging
from itertools import groupby
import snap7
import snap7.util
import config
import file_manager

logger = logging.getLogger(__name__)


class PLCReader(threading.Thread):
    """
    Hilo de lectura continua de un PLC Siemens S7-1200.

    Hereda de threading.Thread. Cada instancia gestiona la conexión
    y el polling de señales de un PLC concreto de forma autónoma —
    incluyendo conexión inicial, reconexión y desconexión.

    Args:
        senales (list): Lista de señales a leer (dict con nombre, db, offset, bytes).
        ip (str): IP del PLC.
        rack (int): Rack del PLC.
        slot (int): Slot del PLC.
    """

    def __init__(self, senales, ip, rack, slot):
        super().__init__()
        self.daemon = True
        self.senales = senales
        self.ip = ip
        self.rack = rack
        self.slot = slot
        self.cliente = None  # Se crea en _conectar()
        self._stop_event = threading.Event()

        # Precalculado una sola vez — nunca cambia durante la ejecución
        self.grupos_db = self._agrupar_por_db(senales)

    def _agrupar_por_db(self, senales):
        """
        Agrupa las señales por DB y precalcula offset y bytes de cada grupo.

        Args:
            senales (list): Lista de señales desde config.py.

        Returns:
            list: Lista de dicts con db, offset_inicial, total_bytes y senales.
        """
        senales_ordenadas = sorted(senales, key=lambda s: s['db'])
        grupos = []

        for db_num, grupo in groupby(senales_ordenadas, key=lambda s: s['db']):
            senales_del_grupo = list(grupo)
            offset_ini = min(senal['offset'] for senal in senales_del_grupo)
            offset_fin = max(senal['offset'] + senal['bytes'] for senal in senales_del_grupo)
            grupos.append({
                'db': db_num,
                'offset_inicial': offset_ini,
                'total_bytes': offset_fin - offset_ini,
                'senales': senales_del_grupo
            })

        return grupos

    def _conectar(self):
        """
        Intenta conectar al PLC. Si falla, reintenta cada Intervalo_reconexion
        segundos hasta conseguirlo o hasta que se llame a stop().

        Returns:
            bool: True si conectó, False si se activó stop_event.
        """
        while not self._stop_event.is_set():
            try:
                logger.info(f"[{self.ip}] Intentando conectar...")
                self.cliente = snap7.client.Client()
                self.cliente.connect(self.ip, self.rack, self.slot)
                logger.info(f"[{self.ip}] Conectado correctamente")
                return True
            except Exception as e:
                logger.error(f"[{self.ip}] Conexión fallida: {e}")
                time.sleep(config.Intervalo_reconexion)
        return False

    def run(self):
        """Bucle principal de polling. Se ejecuta hasta que se llame a stop()."""
        logger.info(f"[{self.ip}] Hilo iniciado")

        # Conexión inicial — si falla reintenta automáticamente
        if not self._conectar():
            logger.info(f"[{self.ip}] Hilo detenido antes de conectar")
            return

        while not self._stop_event.is_set():
            valores = {}

            for grupo in self.grupos_db:
                try:
                    # 1. Una sola lectura al PLC para todo el bloque del DB
                    datos = self.cliente.db_read(
                        grupo['db'],
                        grupo['offset_inicial'],
                        grupo['total_bytes']
                    )

                    # 2. Extraer cada señal de su posición dentro del bloque
                    for senal in grupo['senales']:
                        valor = snap7.util.get_real(datos, senal['offset'])
                        logger.debug(f"[{self.ip}] {senal['nombre']}: {valor}")
                        valores[senal['nombre']] = valor

                except Exception as e:
                    logger.error(f"[{self.ip}] Error leyendo DB{grupo['db']}: {e}")
                    # Conexión rota — intentar reconectar
                    if not self._conectar():
                        break

            # 3. Guardar todas las señales de esta vuelta en el CSV
            if valores:
                file_manager.guardar_vuelta(self.ip, valores)

            # 4. Esperar el intervalo de polling
            time.sleep(config.Intervalo)

        # Desconexión limpia al salir
        if self.cliente:
            self.cliente.disconnect()
            logger.info(f"[{self.ip}] Desconectado del PLC")

        logger.info(f"[{self.ip}] Hilo detenido")

    def stop(self):
        """Señaliza al hilo para que termine en la próxima iteración."""
        self._stop_event.set()