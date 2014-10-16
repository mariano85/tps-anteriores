/*
 * kernel.c
 *
 *  Created on: 08/10/2014
 *      Author: utnso
 */


#include "kernel.h"




int main(int argc, char **argv){

	archivo_logs = log_create("log_Principal", "kernel.c",1, LOG_LEVEL_TRACE);
	queue_log = log_create("queue_log", "kernel.c",1, LOG_LEVEL_TRACE);//LOG

		 if (argc < 2){
			 log_error(archivo_logs, "No se pasaron parametros.");
			 log_destroy(archivo_logs);
			 return 0;
		 }

		 if (!archivo_configuracion_valido()){
		 	 log_error(archivo_logs, "El archivo de configuracion no tiene todos los campos necesarios");
		 	 config_destroy(config);
		 	 return 0;
		 	 }


		 	log_debug(archivo_logs, "inicia kernel");

			NEW = queue_create(); //COLAS
			READY = queue_create();
			EXIT = queue_create();
			EXEC = queue_create();
			BLOCK = queue_create();

			log_debug(archivo_logs, "colas creadas");
			log_debug(queue_log, "prueba");

				sem_init(&mutexNEW, 0, 1);
				sem_init(&mutexREADY, 0, 1);
				sem_init(&mutexEXEC, 0, 1);
				sem_init(&mutexEXIT, 0, 1);
				sem_init(&mutexBLOCK, 0, 1);

				semaforos = dictionary_create();

				log_info(archivo_logs, "Conectandose con la MSP...");



					//socket_MSP=conectarseMSP();

			//	pthread_t thread1; Seria el hilo para manejar LOADER MARIANO - MARIANO - MARIANO - MARIANO
			//	pthread_t thread2; Seria un hilo para pasar de new a ready - Despues verificamos si lo necesitamos

			// Esta hilo seria para manejar lo que es el planificador

				pthread_t thread3;

			//	int puerto_CONSOLA = config_get_int_value(config, "PUERTO_PROG");
				int puerto_CPU =config_get_int_value(config,"PUERTO_CPU");


				int	iret3 = pthread_create(&thread3,NULL, (void*)conectarse_Planificador,(void*)puerto_CPU); //HILO PCP

					if (iret3) {
						log_error(archivo_logs, "Error en la creacion del hilo Planificador");
						log_destroy(archivo_logs);

						exit(EXIT_FAILURE);

					}


					pthread_join(thread3,NULL);

					puts("hola");


	return EXIT_SUCCESS;

}

bool archivo_configuracion_valido(){


	config = config_create(KERNEL_CONF);

	if (!config_has_property(config, "PUERTO_CONSOLA"))
		return 0;

	if (!config_has_property(config, "PUERTO_CPU"))
			return 0;

	if (!config_has_property(config, "PUERTO_MSP"))
			return 0;

	if (!config_has_property(config, "IP"))
				return 0;

	if (!config_has_property(config, "QUANTUM"))
					return 0;

	if (!config_has_property(config, "MULTIPROG"))
						return 0;

	return 1;
}


void conectarse_Planificador(){


	puts("Hola soy un hilo y ando jajajaj");





}

bool _cpuLibre(void* element){

		if(((t_client_cpu*)element)->ocupado == false){
			return true;
		}
		return false;
	}




