import os
import configparser
import yaml
import generate_maps.generate_maps


#Main function
def main():
    # Seleccionar archivo de configuración basado en el sistema operativo
    if os.name == 'posix':  # Linux
        with open('config.yaml', 'r') as infile:
            options = yaml.safe_load(infile)
    elif os.name == 'nt':  # Windows
        config = configparser.ConfigParser()
        config.read('config.ini')
        options = {section: dict(config[section]) for section in config.sections()}

    # Elegir un conjunto de opciones
    chosen_options = options['OPTIONS_MAX']
    generar_grafico(data=pd.read_csv(chosen_options['data_csv']), es_max=chosen_options['es_max'], niveles=chosen_options['levels'], 
                    tiempo=chosen_options['time'], lat_range=chosen_options['lat_range'], lon_range=chosen_options['lon_range'])

    chosen_options = options['OPTIONS_MIN']
    generar_grafico(data=pd.read_csv(chosen_options['data_csv']), es_max=chosen_options['es_max'], niveles=chosen_options['levels'], 
                    tiempo=chosen_options['time'], lat_range=chosen_options['lat_range'], lon_range=chosen_options['lon_range'])

if __name__ == '__main__':
    main()
    
#@TO-DO: interfaz visual para pedir todos los datos necesarios para crear los config files ??? O para llamar a la función y ya.