import snap7 #Para conectar y leer el PLC
import time # Para el time.sleep(Intervalo) al final de cada vuelta del bucle.
import config as config # # Archivo de configuración (IP, RACK, SLOT, Senales)
import plc_reader as plc_reader
#Mirar si tengo que añadir la de MOdbus entre otras
# Mirar las siguientes librerías si fuesen necesarias:


#Creamos una lista donde estarán los objetos que representan diferentes hilos
lectores = []
clientes = []

for plc in config.PLCs:
    # Realizamos la conexion con el cliente
    try:
        cliente = snap7.client.Client()
        cliente.connect(plc['ip'],plc['rack'], plc['slot'])
        print(cliente.get_connected())
        print(cliente.protocol)
    except Exception as e:
        print('No se pudo conectar con el PLC: ',e)
        continue
    
    
    lector = plc_reader.PLCReader(cliente,plc['senales'],plc['ip'])
    lector.start()
    lectores.append(lector)

    clientes.append(cliente)


""" PLC1= plc_reader.PLCReader(cliente,config.Senales)
PLC1.start() """

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt :
    print('Hubo una interrupción de Keyboard con Ctrl +  C')
    #Para recibir toda la ventana temporal, hacemos que el hilo pare, por lo que saldrá del while.
    # Para asegurarnos que el hilo termine usamos luego join()
    for lector in lectores:
        lector.stop()
    for lector in lectores:
        lector.join()
    for client in clientes:
        client.disconnect()
        print("Desconectado del PLC")



