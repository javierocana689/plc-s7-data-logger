# FICHERO DE CONFIGURACIÓN

PLCs = [
    {
        'ip': "***.***.*.**",
        'rack': 0,
        'slot': 1,
        'senales': [
            {'nombre': 'Temperatura', 'db': 1, 'offset': 0, 'bytes': 4},
            {'nombre': 'Velocidad',   'db': 1, 'offset': 4, 'bytes': 4},
            {'nombre': 'Presion',     'db': 1, 'offset': 8, 'bytes': 4},
        ]
    },
]

# Intervalo de polling en segundos
Intervalo = 1.0

# Segundos entre intentos de reconexión
Intervalo_reconexion = 5.0
