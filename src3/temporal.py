from itertools import groupby

# Simulamos señales de dos DBs distintos para ver cómo funciona
senales = [
    {'nombre': 'Temperatura', 'db': 1, 'offset': 0, 'bytes': 4},
    {'nombre': 'Velocidad',   'db': 1, 'offset': 4, 'bytes': 4},
    {'nombre': 'Nivel',       'db': 2, 'offset': 0, 'bytes': 4},
    {'nombre': 'Presion',     'db': 2, 'offset': 4, 'bytes': 4},
]

# Paso 1 — ordenar por db
senales_ordenadas = sorted(senales, key=lambda s: s['db'])

db_senales= []

# Paso 2 — agrupar por db
for db_num, grupo in groupby(senales_ordenadas, key=lambda s: s['db']):
    
    senales_del_grupo = list(grupo)
    offset_ini= min(senal['offset'] for senal in senales_del_grupo)
    offset_fin= max(senal['offset'] + senal['bytes'] for senal in senales_del_grupo)
    bytes_totales = offset_fin - offset_ini
    db_senales.append({'db': db_num,
                      'offset_inicial': offset_ini,
                        'total_bytes': bytes_totales,
                        'senales': senales_del_grupo })

print(db_senales)