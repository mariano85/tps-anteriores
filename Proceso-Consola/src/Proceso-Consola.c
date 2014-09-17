/*
 ============================================================================
 Name        : Proceso-Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "variables_Globales.h"

int main(int argc, char **argv){

	logs = log_create("log", "program.c", 1, LOG_LEVEL_TRACE);
		if (argc < 2){
			log_error(logs, "No se paso el archivo con el codigo BESO");
			liberar_estructuras();
			return 0;
		}

		if (!archivo_de_configuracion_valido()){
				log_error(logs, "Archivo de configuracion incompleto");
			liberar_estructuras();
				return 0;
			}

		socketKernel = conectar_Kernel();
			if (socketKernel < 0){
				log_error(logs, "El programa finalizo, no se pudo conectar con el kernel");
				liberar_estructuras();
				return 0;}

		log_info(logs, "El proceso se conecto correctamente con el kernel");

//	enviar_archivo_al_kernel(argv[1]); // Mando el archivo con el codigo BESO
// Tengo una duda de como se levanta el archivo con el codigo BESO, P R E G U N T A R


	//	cerrarSocket(socketKernel);

			liberar_estructuras();

	return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////Funciones/////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

int conectar_Kernel(){




   return 0;

}

void enviar_archivo_al_kernel(char* archivo){






}

/* Libero la estructuras utilizadas	*/

void liberar_estructuras(){
	config_destroy(config);
	log_destroy(logs);
}


/* Verifico que el archivo de configuracion sea valido */
int archivo_de_configuracion_valido(){
	if (!config_has_property(config, "IP_KERNEL"))
		return 0;
	if (!config_has_property(config, "PUERTO_KERNEL"))
		return 0;
	return 1;
}

