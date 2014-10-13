/*
* kernel.c
 *
 *  Created on: 08/10/2014
 *      Author: utnso
 */


#include "kernel.h"





int main(int argc, char **argv){

	tcb1 = malloc(sizeof(TCB));
	tcb1->pid = 32;
	tcb2 = malloc(sizeof(TCB));
	tcb2->pid = 35;



	kernel_log = log_create("log_Principal", "kernel.c",1, LOG_LEVEL_TRACE);
	queue_log = log_create("queue_log", "kernel.c",1, LOG_LEVEL_TRACE);//LOG

		 if (argc < 2){
			 log_error(kernel_log, "No se pasaron parametros.");
			 log_destroy(kernel_log);
			 return 0;
		 }

		 if (!archivo_configuracion_valido()){
		 	 log_error(kernel_log, "El archivo de configuracion no tiene todos los campos necesarios");
		 	 config_destroy(config);
		 	 return 0;
		 	 }


		 	log_debug(kernel_log, "inicia kernel");

			NEW = queue_create(); //COLAS
			READY = queue_create();
			EXIT = queue_create();
			EXEC = queue_create();
			BLOCK = queue_create();

			log_debug(kernel_log, "colas creadas");
			log_debug(queue_log, "prueba");

				sem_init(&mutexNEW, 0, 1);
				sem_init(&mutexREADY, 0, 1);
				sem_init(&mutexEXEC, 0, 1);
				sem_init(&mutexEXIT, 0, 1);
				sem_init(&mutexBLOCK, 0, 1);

					pthread_mutex_init(&mutex_new_queue, NULL );
					pthread_mutex_init(&mutex_ready_queue, NULL );
					pthread_mutex_init(&mutex_block_queue, NULL );
					pthread_mutex_init(&mutex_exec_queue, NULL );
					pthread_mutex_init(&mutex_exit_queue, NULL );

				semaforos = dictionary_create();

				log_info(kernel_log, "Conectandose con la MSP...");

				agregarProcesoColaNew(tcb1);
				agregarProcesoColaReady(tcb1);
				agregarProcesoColaNew(tcb2);
				agregarProcesoColaReady(tcb2);
				mostrarColas();


					//socket_MSP=conectarseMSP();

			//	pthread_t thread1; Seria el hilo para manejar LOADER MARIANO - MARIANO - MARIANO - MARIANO
			//	pthread_t thread2; Seria un hilo para pasar de new a ready - Despues verificamos si lo necesitamos

			// Esta hilo seria para manejar lo que es el planificador

				pthread_t thread3;

			//	int puerto_CONSOLA = config_get_int_value(config, "PUERTO_PROG");
				int puerto_CPU =config_get_int_value(config,"PUERTO_CPU");


				int	iret3 = pthread_create(&thread3,NULL, (void*)conectarse_Planificador,(void*)puerto_CPU); //HILO PCP

					if (iret3) {
						log_error(kernel_log, "Error en la creacion del hilo Planificador");
						log_destroy(kernel_log);

						exit(EXIT_FAILURE);

					}


					pthread_join(thread3,NULL);

					puts("hola");
					mostrarColas();
					free(tcb1);
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



	return 1;
}


void conectarse_Planificador(){


	puts("Hola soy un hilo y ando jajajaj");





}




void ComunicarMuertePrograma(int32_t processFd) {



}

/*void matarProceso(TCB* aProcess){

	if(StillInside(aProcess->pid)){
		//log_debug(kernel_log, ("Se elimina del sistema las estructuras asociadas al proceso con PID: %d", aProcess->pid));

		free(aProcess);
	}
	else{
		aProcess->pid = 0;
	}

}

*/

void enviarAEjecutar(int32_t socketCPU, int32_t  quantum, TCB* aProcess){

		int32_t v1 = aProcess->pid;

		t_contenido mensaje;
		memset(mensaje, 0, sizeof(t_contenido));
		int quantumss = config_get_int_value(config,QUANTUM);
		strcpy(mensaje, string_from_format("[%d, %d]", v1,quantumss));
	//	enviarMensaje(socketCPU, KRN_TO_CPU_PCB, mensaje, kernel_log); NICO HACE ALGO
		log_info(kernel_log, "Se envía un PCB al CPU libre elegido");

}
