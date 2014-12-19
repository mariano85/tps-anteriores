/*
 * kernel.c
 *
 *  Created on: 11/10/2014
 *      Author: utnso
 */

#include "kernel.h"
extern t_log* logKernel;

int main(){

	// TODO: agregar logs para el planificador y para el loader
	system("clear");
	logKernel = log_create(KERNEL_LOG_PATH, "Kernel", 1, LOG_LEVEL_DEBUG);

	// Hello Kernel!
	int kernel_pid = getpid();
	log_info(logKernel, "************** WELCOME TO KERNEL V1.0! (PID: %d) ***************\n", kernel_pid);
	initKernel();

	pthread_create(&loaderThread.tid, NULL, (void*) loader, (void*) &loaderThread);
	pthread_create(&planificadorThread.tid, NULL, (void*) planificador, (void*) &planificadorThread);
	pthread_create(&manejoColaReadyThread.tid, NULL, (void*) manejo_cola_ready, (void*) &manejoColaReadyThread);
	pthread_create(&manejoColaExitThread.tid, NULL, (void*) manejo_cola_exit, (void*) &manejoColaExitThread);

	pthread_join(loaderThread.tid, NULL);
	pthread_join(planificadorThread.tid, NULL);
	pthread_join(manejoColaReadyThread.tid, NULL);
	pthread_join(manejoColaExitThread.tid, NULL);

	finishKernel();
	return EXIT_SUCCESS;
}


void finishKernel(){

	// destruyo semaforos
	pthread_mutex_destroy(&mutex_cpu_list);
	pthread_mutex_destroy(&mutex_ready_queue);
	pthread_mutex_destroy(&mutex_syscall_semaforo);
	pthread_mutex_destroy(&mutex_syscalls_queue);
	pthread_mutex_destroy(&mutex_join_queue);
	pthread_mutex_destroy(&mutex_exit_queue);

	queue_clean_and_destroy_elements(COLA_READY, (void*)free);
	queue_clean_and_destroy_elements(COLA_SYSCALLS, (void*)free);
	queue_clean_and_destroy_elements(COLA_JOIN, (void*)free);
	queue_clean_and_destroy_elements(COLA_EXIT, (void*)free);

	list_destroy(cpu_client_list);

	log_destroy(logKernel);
}

char* getBytesFromFile(FILE* entrada, size_t *tam_archivo) {
	fseek(entrada, 0L, SEEK_END);
	*tam_archivo = ftell(entrada);
	char * literal = (char*) calloc(1, *tam_archivo);
	fseek(entrada, 0L, 0L);

	fread(literal, *tam_archivo, 1, entrada);
	return literal;
}

void initKernel(){

	loadConfig();

	//Inicializa lista de Cpu's
	// esta es mi cola EXEC. WAJA!
	cpu_client_list = list_create();
	mapa_recursos = dictionary_create();

	//Inicializa colas
	COLA_READY = queue_create();
	COLA_EXIT = queue_create();
	COLA_SYSCALLS = queue_create();
	COLA_JOIN = queue_create();

	//Inicializa semaforos
	pthread_mutex_init(&mutex_cpu_list, NULL);
	pthread_mutex_init(&mutex_ready_queue, NULL );
	pthread_mutex_init(&mutex_syscall_semaforo, NULL );
	pthread_mutex_init(&mutex_syscalls_queue, NULL );
	pthread_mutex_init(&mutex_join_queue, NULL );
	pthread_mutex_init(&mutex_exit_queue, NULL );

	/*Se valida que en el sistema exista una instancia de la MSP levantada. Esto es indispensable
	 * para lograr reservar segmentos para posibles clientes programas*/
	 socketMSP = conectarAServidor(config_kernel.IP_MSP, config_kernel.PUERTO_MSP);

	 while (socketMSP == EXIT_FAILURE) {
		 log_info(logKernel, "Despierten a la MSP! Se reintenta conexion en unos segundos ;) \n");
		 sleep(5);
		 socketMSP = conectarAServidor(config_kernel.IP_MSP, config_kernel.PUERTO_MSP);
	 }

	 handshakeMSP();

	 log_info(logKernel, "Se ha establecido conexion con el proceso MSP");

	 crearProcesoKM();
}


void handshakeMSP() {

	t_contenido mensaje;
	memset(mensaje,0,sizeof(t_contenido));
	enviarMensaje(socketMSP, KERNEL_TO_MSP_HANDSHAKE, mensaje, logKernel);

}

t_process* getProcesoDesdeMensaje(char* mensaje){

	t_process* proceso = calloc(sizeof(t_process), 1);
	t_hilo* process_tcb = calloc(sizeof(t_hilo), 1);
	proceso->tcb = process_tcb;
	char** split = string_get_string_as_array(mensaje);

	proceso->tcb->pid = atoi(split[0]);
	proceso->process_fd = atoi(split[0]);
	proceso->tcb->base_stack = solicitarSegmento(process_tcb->pid, config_kernel.TAMANIO_STACK);

	if(proceso->tcb->base_stack == EXIT_FAILURE){
		log_error(logKernel, "No pudieron reservarse los segmentos para el proceso %d", proceso->tcb->pid);
		free(proceso);
		return NULL;
	}

	proceso->tcb->tid = atoi(split[1]);
	proceso->tcb->kernel_mode = atoi(split[2]);
	proceso->tcb->segmento_codigo = atoi(split[3]);
	proceso->tcb->segmento_codigo_size = atoi(split[4]);
	proceso->tcb->puntero_instruccion = atoi(split[5]);
	proceso->tcb->base_stack = atoi(split[6]);
	proceso->tcb->cursor_stack = atoi(split[7]);

	proceso->tcb->registros[0] = atoi(split[8]);
	proceso->tcb->registros[1] = atoi(split[9]);
	proceso->tcb->registros[2] = atoi(split[10]);
	proceso->tcb->registros[3] = atoi(split[11]);
	proceso->tcb->registros[4] = atoi(split[12]);

	return proceso;
}


t_process* getProcesoDesdeCodigoBESO(char* nombreBESO, bool indicadorModo, char* codigoBESO, int32_t tamanioCodigo, int32_t fd)
{
	t_process* proceso = calloc(sizeof(t_process), 1);
	t_hilo* process_tcb = calloc(sizeof(t_hilo), 1);

	process_tcb->pid = fd;
	process_tcb->tid = 0;
	process_tcb->kernel_mode = indicadorModo;
	strcpy(proceso->nombre, nombreBESO);

	log_info(logKernel, "Solicitamos segmento de codigo para pid %d", fd);
	process_tcb->segmento_codigo = solicitarSegmento(process_tcb->pid, tamanioCodigo);
	process_tcb->segmento_codigo_size = tamanioCodigo;

	if(!process_tcb->kernel_mode){
		log_info(logKernel, "Solicitamos segmento de stack para pid %d", fd);
		process_tcb->base_stack = solicitarSegmento(process_tcb->pid, config_kernel.TAMANIO_STACK);
		process_tcb->cursor_stack = process_tcb->base_stack;
	}

	proceso->process_fd = fd;
	proceso->tcb = process_tcb;

	if(process_tcb->segmento_codigo == EXIT_FAILURE || process_tcb->base_stack == EXIT_FAILURE){
		log_error(logKernel, "No pudieron reservarse los segmentos para el proceso %d", fd);
		free(proceso);

		return NULL;
	}

	// TODO: proximamente el header sera MSP_TO_KERNEL_MEMORIA_ESCRITA
	t_header header = escribirMemoria(process_tcb->pid, process_tcb->segmento_codigo, codigoBESO, tamanioCodigo);
	if(header == EXIT_FAILURE){
		log_error(logKernel, "No pudo escribirse en el segmento de codigo del proceso %d", fd);
		free(proceso);

		return NULL;
	}

	log_info(logKernel, "Se generó la estructura del proceso con éxito! NOMBRE: %s; PID: %d", proceso->nombre, fd);
	return proceso;
}

uint32_t solicitarSegmento(int32_t id_proceso, uint32_t tamanio){

	t_contenido msjRespuesta;
	memset(msjRespuesta,0,sizeof(t_contenido));

	enviarMensaje(socketMSP, KERNEL_TO_MSP_MEM_REQ, string_from_format("[%d,%d]", id_proceso, tamanio), logKernel);

	recibirMensaje(socketMSP, msjRespuesta, logKernel);

	return atoi(msjRespuesta);
}

void eliminarSegmento(int32_t id_proceso, uint32_t tamanio){
	t_contenido msjRespuesta;
	memset(msjRespuesta,0,sizeof(t_contenido));

	enviarMensaje(socketMSP, KERNEL_TO_MSP_ELIMINAR_SEGMENTOS, string_from_format("[%d,%d]", id_proceso, tamanio), logKernel);
}

void loadConfig(){

	log_info(logKernel, "Se inicializa el kernel con parametros desde: %s", KERNEL_CONFIG_PATH);
	 kernelConfig = config_create(KERNEL_CONFIG_PATH);

	if (config_has_property(kernelConfig, "PUERTO")) {
		//strcpy(PUERTO_PROG, (char*) config_get_string_value(configPath, "PUERTO_PROG"));
		config_kernel.PUERTO = config_get_int_value(kernelConfig,
				"PUERTO");
	} else {
		log_error(logKernel,
				"No se encontro la key 'PUERTO' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "IP_MSP")) {
		config_kernel.IP_MSP = string_duplicate(
				config_get_string_value(kernelConfig, "IP_MSP"));
	} else {
		log_error(logKernel,
				"No se encontro la key 'IP_MSP' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "PUERTO_MSP")) {
		config_kernel.PUERTO_MSP = config_get_int_value(kernelConfig,
				"PUERTO_MSP");
	} else {
		log_error(logKernel,
				"No se encontro la key 'PUERTO_MSP' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "QUANTUM")) {
		config_kernel.QUANTUM = string_duplicate(
				config_get_string_value(kernelConfig, "QUANTUM"));
	} else {
		log_error(logKernel,
				"No se encontro la key 'QUANTUM' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "SYSCALLS")) {
		config_kernel.SYSCALLS = string_duplicate(
				config_get_string_value(kernelConfig, "SYSCALLS"));
	} else {
		log_error(logKernel,
				"No se encontro la key 'SYSCALLS' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "TAMANIO_STACK")) {
			config_kernel.TAMANIO_STACK = config_get_int_value(kernelConfig,
					"TAMANIO_STACK");
		} else {
			log_error(logKernel,
					"No se encontro la key 'TAMANIO_STACK' en el archivo de configuracion");
			config_destroy(kernelConfig);
			exit(EXIT_FAILURE);
		}

}

void comunicarMuertePrograma(int32_t pid, bool wasInMsp){
	log_info(logKernel, "implementar comunicarMuertePrograma( pid: %d, estabaEnLaMSP: %d", pid, wasInMsp);
}

void eliminarSegmentos(int32_t pid){
	log_info(logKernel, "implementar eliminarSegmentos pid: %d", pid);
}


int32_t escribirMemoria(int32_t pid, uint32_t direccionSegmento, char* buffer, int32_t tamanio){
	t_contenido msjRespuesta;
	memset(msjRespuesta,0,sizeof(t_contenido));

	enviarMensaje(socketMSP, KERNEL_TO_MSP_ENVIAR_BYTES, string_from_format("[%d,%d,%d]", pid, tamanio, direccionSegmento), logKernel);

	recibirMensaje(socketMSP, msjRespuesta, logKernel);
	log_info(logKernel, "respuesta de la msp: %s", msjRespuesta);

	enviar(socketMSP, buffer, tamanio);
	recibirMensaje(socketMSP, msjRespuesta, logKernel);

	memset(msjRespuesta,0,sizeof(t_contenido));

	return atoi(msjRespuesta);
}


void crearProcesoKM(){
	FILE *entrada;
	size_t cantBytes = 0;
	char *bufferArchivoSysCalls = NULL;

	if ((entrada = fopen(config_kernel.SYSCALLS, "r")) == NULL ) {
		perror(config_kernel.SYSCALLS);
		exit(EXIT_FAILURE);
	}

	bufferArchivoSysCalls = getBytesFromFile(entrada, &cantBytes);

	// por ahora le mando el socket de la conexion de la msp, deberia mandarle un 0 (creo)
	// si: el tcb de kernel es una variable global
	procesoKernel = getProcesoDesdeCodigoBESO(config_kernel.SYSCALLS, MODO_KERNEL, bufferArchivoSysCalls, cantBytes, KERNEL_PID);

	if(procesoKernel == NULL){
		log_error(logKernel, "Ocurrio un error al intentar crear el proceso de syscalls. El Kernel aborta...");
		finishKernel();
		exit(EXIT_FAILURE);
	}

	log_info(logKernel, "Bloqueamos el proceso Kernel");

	// voilá! "meto" en la "cola de block" al tcb del Kernel
	setearProcesoCola(procesoKernel, BLOCK);
}
