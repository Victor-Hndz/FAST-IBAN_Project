import sys
import configparser
import yaml
import os

def help():
    print('''
    Este script genera un archivo de configuración para el programa de visualización de datos.
    ''')
    
    print('''
          USO: python config_file_CLI.py MAPA MIN/MAX TIEMPO LAT_MIN LAT_MAX LON_MIN LON_MAX [TIPO] (TIPO solo si el mapa es de seleccionados)
            ''')
    
    print('''
          MAPA: mapa a generar. Puede ser uno de los siguientes:
              - contornos
              - dispersión
              - combinado
              - combinado con círculos
              - de seleccionados
            ''')
    
    print('''
            MIN/MAX: puede ser uno de los siguientes:
                - min
                - max
                - both
                ''')
    
    print('''
                TIEMPO: puede ser uno o más de los siguientes:
                    - 00
                    - 06
                    - 12
                    - 18
                    ''')
    
    print('''
          TIPO: puede ser uno de los siguientes:
              - rex
              - omega
              ''')
    
    print('''
          EJEMPLO: python config_file_CLI.py contornos both 00 25 90 -180 180
          ''')

def main():      
    if len(sys.argv) < 8 or len(sys.argv) > 9 or sys.argv[1] == '-h' or sys.argv[1] == '--help':
        help()
        return
    if sys.argv[1] not in ['contornos', 'dispersión', 'combinado', 'combinado con círculos', 'de seleccionados']:
        print('El mapa debe ser uno de los siguientes: contornos, dispersión, combinado, combinado con círculos, de seleccionados')
        return
    
    if sys.argv[2] not in ['min', 'max', 'both']:
        print('El tipo de mapa debe ser uno de los siguientes: min, max, both')
        return
    
    if sys.argv[3] not in ['00', '06', '12', '18']:
        print('El tiempo debe ser uno de los siguientes: 00, 06, 12, 18')
        return
    
    if sys.argv[1] == 'de seleccionados' and len(sys.argv) != 9:
        print('El mapa de seleccionados requiere un tipo de dato: rex u omega')
        return
    
    if sys.argv[1] != 'de seleccionados' and len(sys.argv) != 8:
        help()
        return
    
    if sys.argv[1] == 'de seleccionados' and sys.argv[8] not in ['rex', 'omega']:
        print('El tipo de dato debe ser uno de los siguientes: rex, omega')
        return
    
    # Seleccionar archivo de configuración basado en el sistema operativo
    if os.name == 'posix':  # Linux
        with open('config.yaml', 'r') as infile:
            options = yaml.safe_load(infile)
    elif os.name == 'nt':  # Windows
        config = configparser.ConfigParser()
        config.read('config.ini')
        options = {section: dict(config[section]) for section in config.sections()}
        
    # Elegir un conjunto de opciones
    chosen_options = options['OPTIONS']
    
    # Elegir el mapa
    
    

if __name__ == '__main__':
    main()
    