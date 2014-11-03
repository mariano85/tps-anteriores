/*
 * kernel.c
 *
 *  Created on: 11/10/2014
 *      Author: utnso
 */

#include "kernel.h"
extern t_log* logKernel;


int main(){

	///Crea un archivo de log para el kernel
	logKernel = log_create(KERNEL_LOG_PATH, "Kernel", 1, LOG_LEVEL_DEBUG);
	//Crea un archivo para log de colas
	queueLog = log_create(QUEUE_LOG_PATH, "Kernel - Queues", 1, LOG_LEVEL_INFO);

	// Hello Kernel!
	//system("clear");
	//int kernel_pid = getpid();
	//log_info(logKernel, "************** WELCOME TO KERNEL V1.0! (PID: %d) ***************\n", kernel_pid);
	initKernel();

	// Conexion CPU Y HANDSHAKE

	socket_cpu = conectarAServidor(config_kernel.IP_CPU,config_kernel.PUERTO);

	while(socket_cpu == EXIT_FAILURE){
		log_info(logKernel,"Despierten al CPU! Se reintenta conexion en unos segundos \n");
			sleep(22);
		socket_cpu = conectarAServidor(config_kernel.IP_CPU,config_kernel.PUERTO);

	}

	//1) Fase uno es el handshake

		t_contenido mensaje;
		memset(mensaje,0,sizeof(t_contenido));
		strcpy(mensaje,"hola");

		enviarMensaje(socket_cpu,KERNEL_TO_CPU_HANDSHAKE,mensaje,logKernel);



//	pthread_create(&loaderThread.tid, NULL, (void*) loader, (void*) &loaderThread);
	pthread_create(&planificadorThread.tid, NULL, (void*) planificador, (void*) &loaderThread);
	pthread_create(&manejoColaReadyThread.tid, NULL, (void*) manejo_cola_ready, (void*) &loaderThread);
//	pthread_create(&manejoColaExitThread.tid, NULL, (void*) manejo_cola_exit, (void*) &loaderThread);

//	pthread_join(loaderThread.tid, NULL);
	pthread_join(planificadorThread.tid, NULL);
	pthread_join(manejoColaReadyThread.tid, NULL);
//	pthread_join(manejoColaExitThread.tid, NULL);
	finishKernel();

	return EXIT_SUCCESS;
}


void finishKernel(){
	log_destroy(logKernel);
	log_destroy(queueLog);
}

void initKernel(){

	loadConfig();

	//Inicializa lista de Cpu's
	cpu_disponibles_list = list_create();

	pthread_mutex_init(&mutex_cpu_list, NULL);

	//Inicializa colas
	NEW = queue_create();
	READY = queue_create();
	BLOCK = queue_create();
	EXEC = queue_create();
	EXIT = queue_create();

	//Inicializa semaforo de colas
	pthread_mutex_init(&mutex_new_queue, NULL );
	pthread_mutex_init(&mutex_ready_queue, NULL );
	pthread_mutex_init(&mutex_block_queue, NULL );
	pthread_mutex_init(&mutex_exec_queue, NULL );
	pthread_mutex_init(&mutex_exit_queue, NULL );




	//******************************************
	//********************************************************//PRUEBA

			t_process* aProcess = malloc(sizeof(t_process));
			t_process* aProcess2 = malloc(sizeof(t_process));
			t_process* aProcess3 = malloc(sizeof(t_process));
			t_tcb* aTcb1 = malloc(sizeof(t_tcb));
			t_tcb* aTcb2 = malloc(sizeof(t_tcb));
			t_tcb* aTcb3 = malloc(sizeof(t_tcb));
			aProcess->tcb = aTcb1;
			aProcess2->tcb = aTcb2;
			aProcess3->tcb = aTcb3;
			aTcb1->pid = 1024;
			aTcb1->indicador_modo_kernel = 0;
			aTcb3->pid = 1025;
			aTcb3->indicador_modo_kernel = 0;
			agregarProcesoColaNew(aProcess);
			agregarProcesoColaNew(aProcess2);
			agregarProcesoColaNew(aProcess3);


			//ESTE ES UN PROCESO QUE TENGO QUE FINALIZAR




/*


	 socketMSP = conectarAServidor(config_kernel.IP_MSP, config_kernel.PUERTO_MSP);

	 while (socketMSP == EXIT_FAILURE) {
		 log_info(logKernel, "Despierten a la MSP! Se reintenta conexion en unos segundos ;) \n");
		 sleep(5);
		 socketMSP = conectarAServidor(config_kernel.IP_MSP, config_kernel.PUERTO_MSP);
	 }

	 handshakeMSP();

	 log_info(logKernel, "Se ha establecido conexion con el proceso MSP\n");

	 */

}


t_process* getProcesoDesdeCodigoBESO(int32_t indicadorModo, char* codigoBESO, int32_t tamanioCodigo, int32_t PID, int32_t TID, int32_t fd)
{
	t_process* proceso = calloc(sizeof(t_process), 1);
	t_tcb* process_tcb = calloc(sizeof(t_tcb), 1);

	process_tcb->pid = PID;
	process_tcb->tid = TID;
	process_tcb->indicador_modo_kernel = indicadorModo;

	process_tcb->base_segmento_codigo = solicitarSegmento(process_tcb->pid, tamanioCodigo);
	process_tcb->base_stack = solicitarSegmento(process_tcb->pid, config_kernel.TAMANIO_STACK);

	proceso->process_fd = fd;
	proceso->tcb = process_tcb;

	if(process_tcb->base_segmento_codigo == EXIT_FAILURE || process_tcb->base_stack == EXIT_FAILURE){
		log_error(logKernel, "No pudieron reservarse los segmentos para el proceso %d", PID);
		free(proceso);

		return NULL;
	}

	if(escribirMemoria(process_tcb->pid, process_tcb->base_segmento_codigo, codigoBESO, tamanioCodigo) == EXIT_FAILURE){
		log_error(logKernel, "No pudo escribirse en el segmento de codigo del proceso %d", PID);
		free(proceso);

		return NULL;
	}

	return proceso;
}

void crearProcesoKM(){
	t_process* proceso = NULL;
	FILE *entrada;
	size_t cantBytes = 0;
	char *bufferArchivoSysCalls = NULL;

	if ((entrada = fopen(config_kernel.SYSCALLS, "r")) == NULL ) {
		perror(config_kernel.SYSCALLS);
		exit(EXIT_FAILURE);
	}

	bufferArchivoSysCalls = getBytesFromFile(entrada, &cantBytes);

	// por ahora le mando el socket de la conexcion de la msp, deberia mandarle un 0 (creo)
	proceso = getProcesoDesdeCodigoBESO(MODO_KERNEL, bufferArchivoSysCalls, cantBytes, SYS_CALLS_PID, SYS_CALLS_TID, socketMSP);

	if(proceso == NULL){
		log_error(logKernel, "Ocurrio un error al intentar crear el proceso de syscalls. El Kernel aborta...");

		exit(EXIT_FAILURE);
	}

	agregarProcesoKernel(proceso);
}

char* getBytesFromFile(FILE* entrada, size_t *tam_archivo) {
	fseek(entrada, 0L, SEEK_END);
	*tam_archivo = ftell(entrada);
	char * literal = (char*) calloc(1, *tam_archivo);
	fseek(entrada, 0L, 0L);

	fgets(literal, *tam_archivo, entrada);
	return literal;
}

int32_t escribirMemoria(int32_t pid, uint32_t direccionSegmento, char* buffer, int32_t tamanio){
	t_contenido msjRespuesta;
	memset(msjRespuesta,0,sizeof(t_contenido));

	enviarMensaje(socketMSP, KERNEL_TO_MSP_ENVIAR_BYTES, string_from_format("[%d,%d,%d]", pid, tamanio, direccionSegmento), logKernel);

	recibirMensaje(socketMSP, msjRespuesta, logKernel);
	log_info(logKernel, "respuesta de la msp: %s", msjRespuesta);

	enviarMensaje(socketMSP, KERNEL_TO_MSP_ENVIAR_BYTES, buffer, logKernel);
	recibirMensaje(socketMSP, msjRespuesta, logKernel);

	memset(msjRespuesta,0,sizeof(t_contenido));

	return atoi(msjRespuesta);
}

uint32_t solicitarSegmento(int32_t id_proceso, uint32_t tamanio){

	t_contenido msjRespuesta;
	int32_t direccionMSP = EXIT_FAILURE;
	memset(msjRespuesta,0,sizeof(t_contenido));


	enviarMensaje(socketMSP, KERNEL_TO_MSP_MEM_REQ, string_from_format("[%d,%d]", id_proceso, tamanio), logKernel);

	t_header header = recibirMensaje(socketMSP, msjRespuesta, logKernel);

	if(header == MSP_TO_KERNEL_SEGMENTO_CREADO){
		direccionMSP = atoi(msjRespuesta);
	} else {
		log_error(logKernel, "no deberia recibir el header MSP_TO_KERNEL_MEM_REQ ???");
	}

	return direccionMSP;
}


void handshakeMSP() {

	t_contenido mensaje;
	// deberiamos formatear el mensaje todo en 0's
	enviarMensaje(socketMSP, KERNEL_TO_MSP_HANDSHAKE, mensaje, logKernel);

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

	if (config_has_property(kernelConfig, "IP_CPU")) {
		config_kernel.IP_CPU = string_duplicate(
				config_get_string_value(kernelConfig, "IP_CPU"));
	} else {
		log_error(logKernel,
				"No se encontro la key 'IP_CPU' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "PUERTO_CPU")) {
		config_kernel.PUERTO_CPU = config_get_int_value(kernelConfig,
				"PUERTO_CPU");
	} else {
		log_error(logKernel,
				"No se encontro la key 'PUERTO_CPU' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "QUANTUM")) {
		config_kernel.PUERTO_CPU = config_get_int_value(kernelConfig,
				"QUANTUM");
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

}

void comunicarMuertePrograma(int32_t pid, bool wasInMsp){
	log_info(logKernel, "implementar comunicarMuertePrograma( pid: %d, estabaEnLaMSP: %d", pid, wasInMsp);
}

void eliminarSegmentos(int32_t pid){
	log_info(logKernel, "implementar eliminarSegmentos pid: %d", pid);
}

void killProcess(t_process* aProcess){

	if(stillInside(aProcess->process_fd)){
			log_info(logKernel, string_from_format("Se elimina del sistema las estructuras asociadas al proceso con PID: %d", aProcess->tcb->pid));
			free(aProcess->tcb);
			free(aProcess);
		}
		else{
			aProcess->process_fd = 0;
		}
	log_info(logKernel, "implementar killProcess pid: %d", aProcess->tcb->pid);
}


t_client_cpu* encontrarCPUporFd(int32_t cpuFd){

	bool _match_cpu_fd(void* element){
		if(((t_client_cpu*)element)->cpuFD == cpuFd){
			return true;
		}
		return false;
	}

	return list_find(cpu_client_list, (void*)_match_cpu_fd);
}



void enviarAEjecutar(int32_t socketCPU, int32_t  quantum, t_process* aProcess){

				t_contenido mensaje;
				memset(mensaje, 0, sizeof(t_contenido));
				strcpy(mensaje, string_from_format("[%d, %d, %d,%d,%d]", aProcess->tcb->pid,aProcess->tcb->base_segmento_codigo, aProcess->tcb->tamanio_segmento_codigo, config_kernel.QUANTUM,aProcess->tcb->indicador_modo_kernel));
				enviarMensaje(socketCPU, KERNEL_TO_CPU_TCB, mensaje, logKernel);
				log_info(logKernel, "Envie un tcb a la cpu elegida. AHORA LO TIENE QUE EJECUTAR !!!!!!!!!");

}




