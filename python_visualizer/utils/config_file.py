import configparser
import yaml
import os
import pandas as pd

# Tus opciones
options_max = {
    'levels': 70,
    'time': 0,
    'lat_range': [25, 90],
    'lon_range': [-180, 180],
    'data_csv': 'data/Geopotential_selected_max.csv',
    'es_max': True
}

options_min = {
    'levels': 70,
    'time': 0,
    'lat_range': [25, 90],
    'lon_range': [-180, 180],
    'data_csv': 'data/Geopotential_selected_min.csv',
    'es_max': False
}

# Crear archivo .ini
config = configparser.ConfigParser()
config['OPTIONS_MAX'] = options_max
config['OPTIONS_MIN'] = options_min
with open('../config.ini', 'w') as configfile:
    config.write(configfile)

# Crear archivo .yaml
options = {'OPTIONS_MAX': options_max, 'OPTIONS_MIN': options_min}
with open('../config.yaml', 'w') as outfile:
    yaml.dump(options, outfile, default_flow_style=False)
