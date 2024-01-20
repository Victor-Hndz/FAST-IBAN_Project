import os
import configparser
import yaml
import pandas as pd
from generate_maps import generate_maps as gm


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
        
    if not os.path.exists('out'):
        os.makedirs('out')

    
    for option in options:           
        if options[option]['map'] == 'cont':
            print('Generating contour map')
            if options[option]['es_max'] == 'both':
                gm.generate_contour_map(nc_data=options[option]['nc_data'], es_max=True, niveles=options[option]['levels'],
                                        tiempo=options[option]['time'], lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
                
                gm.generate_contour_map(nc_data=options[option]['nc_data'], es_max=False, niveles=options[option]['levels'],
                                        tiempo=options[option]['time'], lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
            else:
                gm.generate_contour_map(nc_data=options[option]['nc_data'], es_max=options[option]['es_max'], niveles=options[option]['levels'],
                                        tiempo=options[option]['time'], lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])

        elif options[option]['map'] == 'disp':
            print('Generating scatter map')
            if options[option]['es_max'] == 'both':
                gm.generate_scatter_map(data=pd.read_csv('data/Geopotential_selected_max.csv'), es_max=True, tiempo=options[option]['time'],
                                        lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
                
                gm.generate_scatter_map(data=pd.read_csv('data/Geopotential_selected_min.csv'), es_max=False, tiempo=options[option]['time'],
                                        lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
            else:
                gm.generate_scatter_map(data=pd.read_csv('data/Geopotential_selected_'+options[option]['es_max']+'.csv'), es_max=options[option]['es_max'], tiempo=options[option]['time'],
                                        lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
                
        elif options[option]['map'] == 'comb':
            print('Generating combined map')
            if options[option]['es_max'] == 'both':
                gm.generate_combined_map(data=pd.read_csv('data/Geopotential_selected_max.csv'), nc_data=options[option]['nc_data'], 
                                         es_max=True, niveles=options[option]['levels'], tiempo=options[option]['time'], 
                                         lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
                
                gm.generate_combined_map(data=pd.read_csv('data/Geopotential_selected_min.csv'), nc_data=options[option]['nc_data'], 
                                         es_max=False, niveles=options[option]['levels'], tiempo=options[option]['time'], 
                                         lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
            else:
                gm.generate_combined_map(data=pd.read_csv('data/Geopotential_selected_'+options[option]['es_max']+'.csv'), nc_data=options[option]['nc_data'], 
                                         es_max=options[option]['es_max'], niveles=options[option]['levels'], tiempo=options[option]['time'], 
                                         lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
        
        elif options[option]['map'] == 'comb_circ':
            print('Generating combined map with circles')
            if options[option]['es_max'] == 'both':
                gm.generate_combined_map_circle(data=pd.read_csv('data/Geopotential_selected_max.csv'), nc_data=options[option]['nc_data'],
                                                es_max=True, niveles=options[option]['levels'], tiempo=options[option]['time'],
                                                lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
                
                gm.generate_combined_map_circle(data=pd.read_csv('data/Geopotential_selected_min.csv'), nc_data=options[option]['nc_data'],
                                                es_max=False, niveles=options[option]['levels'], tiempo=options[option]['time'],
                                                lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
            else:
                gm.generate_combined_map_circle(data=pd.read_csv('data/Geopotential_selected_'+options[option]['es_max']+'.csv'), nc_data=options[option]['nc_data'],
                                                es_max=options[option]['es_max'], niveles=options[option]['levels'], tiempo=options[option]['time'],
                                                lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
                
        elif options[option]['map'] == 'select':
            print('Generating selected map')
            if options[option]['type'] == 'both':
                gm.generate_scatter_map_selected(data=pd.read_csv('data/Geopotential_candidates.csv'), tipo='omega',
                                             tiempo=options[option]['time'], lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
                
                gm.generate_scatter_map_selected(data=pd.read_csv('data/Geopotential_candidates.csv'), tipo='rex',
                                             tiempo=options[option]['time'], lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
            else:
                gm.generate_scatter_map_selected(data=pd.read_csv('data/Geopotential_candidates.csv'), tipo=options[option]['type'],
                                             tiempo=options[option]['time'], lat_range=options[option]['lat_range'], lon_range=options[option]['lon_range'])
                
                
if __name__ == '__main__':
    main()
    
#@TO-DO: interfaz visual para pedir todos los datos necesarios para crear los config files ??? O para llamar a la función y ya.