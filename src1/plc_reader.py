import threading
import time
import snap7 #Para conectar y leer el PLC
import config as config # # Archivo de configuración (IP, RACK, SLOT, Senales) Solo sacamos Intervalo
import file_manager as file_manager

'''
Clase PLCReader:
    Clase que generaliza un PLC por su cliente, y como objeto PLC solo necesita
    los parámetros db, offset y bytes para lectura.
    Al ser generalizado podríamos leer N PLCs distintos dependiendo del parámetro
    cliente, el cual fue construido con (IP, rack y slot)
    ¿Si está generalizado solo deberían entrar los campos de la lista de diccionarios de config?
'''
class PLCReader(threading.Thread):
    def __init__(self, cliente, senales, ip):
        super().__init__()
        self.daemon = True # Muere con el programa
        self.cliente = cliente # Pasamos la conexión realizada con ese PLC concreto (IP+ RACK + SLOT)
        self.senales = senales
        self.ip = ip
        self._stop_event = threading.Event()
        '''
        threading.Event() es como un interruptor que puede estar en dos estados:
        apagado → is_set() devuelve False
        encendido → is_set() devuelve True
        Por defecto empieza apagado, cuando llamemos al método set() lo encendemos
        '''

    # Queremos realizar lecturas consecutivas por señal. Por lo que señal=hilo?
    '''
    1.) Leemos los datos del PLC, llegan en bytes
    2.)Realizamos la conversión de bytes a real para así poder obtener el dato real
    3.)#Mostramos el valor con su índice
    4...ultimo)Creamos una carpeta diaria donde creemos un fichero de texto y introducimos señal por señal
    Ultimo paso)Damos paso al siguiente hilo con time sleep
    '''
    def run(self):
        
        #Abrimos paso a captación de errores 
        #try:
            # Recorremos X PLCs
        #Para asegurar que se cargará toda una ventana temporal de datos de un PLC lo aseguramos con un graceful shutdown 
        while not self._stop_event.is_set(): # mientras el interruptor esté apagado, sigue
            # Recorro las n señales de X PLC
            valores = {}
            for senal in self.senales:
                #Comprobamos si podemos leer X señal
                try:
                    #Leemos los datos del PLC:
                    datos = self.cliente.db_read(senal['db'], senal['offset'], senal['bytes'])
                    #Realizamos la conversión de bytes a real para así poder obtener el dato real
                    valor = snap7.util.get_real(datos, 0)
                    #Mostramos el valor con su índice
                    print(senal['nombre'], valor)
                    valores[senal['nombre']] = valor
                except Exception as e:
                    # Hubo algún error
                    print('Error al leer del PLC: ', e)

            #guardar todos los datos de la vuelta de una sola vez
            file_manager.guardar_vuelta(self.ip, valores)
            #Esperamos 1 segundo fuera del for
            time.sleep(config.Intervalo)


    # Este es el método que me permite 
    def stop(self):
        self._stop_event.set()


