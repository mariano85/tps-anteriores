/*
 * msp.c
 *
 *  Created on: 26/09/2014
 *      Author: utnso
 */
#include "msp.h"
#include <commons/config.h>
#include <stdlib.h>
#include <stdio.h>

t_configuracion levantarArchivoDeConfiguracion()
{
 	int puerto;
	int tamanio;
	int swap;
	char* algoritmo;
	t_configuracion configuracion;


	/*Compruebo si existe el archivo (tal vez no sea la manera más
	 * eficiente).
	 */
	FILE* file;

	file = fopen("configuracion", "r");

	if(file == NULL )
	{
		puts("El archivo no existe!");
		exit(0);

	}
	else
	{
		puts("El archivo existe!");
		fclose(file);
	}


	//Abro el archivo de configuracion
	t_config *config;

	config = malloc(sizeof(t_config));

	char* path = "configuracion";

	config = config_create(path);

	//Pido los valores de configuracion y los vuelco en una estructura
	algoritmo = config_get_string_value(config, "SUST_PAGS");
	puerto = config_get_int_value(config, "PUERTO");
	tamanio = config_get_int_value(config, "CANTIDAD_MEMORIA");
	swap = config_get_int_value(config, "CANTIDAD_SWAP");

	configuracion.sust_pags = algoritmo;
	configuracion.puerto = puerto;
	configuracion.cantidad_memoria = tamanio;
	configuracion.cantidad_swap = swap;

	free(config);

	//Devuelvo la estructura cuyos campos son los parametros de configuracion
	return configuracion;

}


