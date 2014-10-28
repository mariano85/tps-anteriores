/*
 * kernel.c
 *
 *  Created on: 11/10/2014
 *      Author: utnso
 */

#include "kernel.h"
extern t_log* logKernel;
t_log* queueLog;

int main(){

	//Crea un archivo de log para el kernel
	logKernel = log_create(KERNEL_LOG_PATH, "Kernel", true, LOG_LEVEL_DEBUG);
	//Crea un archivo para log de colas
	queueLog = log_create(QUEUE_LOG_PATH, "Kernel - Queues", false, LOG_LEVEL_INFO);

	// Hello Kernel!
	//system("clear");
	int kernel_pid = getpid();
	log_info(logKernel, "************** WELCOME TO KERNEL V1.0! (PID: %d) ***************\n", kernel_pid);

	initKernel();

	// Conexion CPU Y HANDSHAKE

		int socket_cpu = conectarAServidor(config_kernel.IP_CPU,config_kernel.PUERTO);

		while(socket_cpu == EXIT_FAILURE){
			log_info(logKernel,"Despierten al CPU! Se reintenta conexion en unos segundos \n");
				sleep(15);
			socket_cpu = conectarAServidor(config_kernel.IP_CPU,config_kernel.PUERTO);

		}


		//1) Fase uno es el handshake

			t_contenido mensaje;
			memset(mensaje,0,sizeof(t_contenido));
			strcpy(mensaje,"hola");

			enviarMensaje(socket_cpu,KERNEL_TO_CPU_HANDSHAKE,mensaje,logKernel);

		//	t_process *processPrueba = processPrueba;

		//	processPrueba->tcb->pid = 1234;
		//	processPrueba->tcb->program_counter = 1048576;

			int32_t pid = 1234;
			int32_t program_counter = 1048576;



			t_contenido mensaje_1;
			memset(mensaje_1, 0, sizeof(t_contenido));
			strcpy(mensaje_1, string_from_format("[%d,%d]", pid,program_counter));
			enviarMensaje(socket_cpu, KERNEL_TO_CPU_TCB, mensaje_1, logKernel);
			log_info(logKernel, "Se envía un TCB al CPU libre elegido");


	pthread_create(&loaderThread.tid, NULL, (void*) loader, (void*) &loaderThread);
	pthread_create(&planificadorThread.tid, NULL, (void*) planificador, (void*) &loaderThread);	pthread_create(&manejoColaReadyThread.tid, NULL, (void*) manejo_cola_ready, (void*) &loaderThread);
	pthread_create(&manejoColaReadyThread.tid, NULL, (void*) manejo_cola_ready, (void*) &loaderThread);
	pthread_create(&manejoColaExitThread.tid, NULL, (void*) manejo_cola_exit, (void*) &loaderThread);

	pthread_join(loaderThread.tid, NULL);
	pthread_join(planificadorThread.tid, NULL);
	pthread_join(manejoColaReadyThread.tid, NULL);
	pthread_join(manejoColaExitThread.tid, NULL);
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
	cpu_client_list = list_create();

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

	socketMSP = conectarAServidor(config_kernel.IP_MSP, config_kernel.PUERTO_MSP);

	 while (socketMSP == EXIT_FAILURE) {
		 log_info(logKernel, "Despierten a la MSP! Se reintenta conexion en unos segundos ;) \n");
		 sleep(5);
		 socketMSP = conectarAServidor(config_kernel.IP_MSP, config_kernel.PUERTO_MSP);
	 }

	 log_info(logKernel, "Logro conectarse a la MSP! =)");

	 handshakeMSP();

	 log_info(logKernel, "Se ha establecido conexion con el proceso MSP\n");

}

void handshakeMSP() {

	t_contenido mensaje;
	// deberiamos formatear el mensaje todo en 0's
	enviarMensaje(socketMSP, KERNEL_TO_MSP_HANDSHAKE, mensaje, logKernel);

}

void loadConfig(){

	log_info(logKernel, "Se inicializa el kernel con parametros desde: %s", KERNEL_CONFIG_PATH);
	t_config* kernelConfig = config_create(KERNEL_CONFIG_PATH);

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


t_client_cpu* GetCPUByCPUFd(int32_t cpuFd){

	bool _match_cpu_fd(void* element){
		if(((t_client_cpu*)element)->cpuFD == cpuFd){
			return true;
		}
		return false;
	}

	return list_find(cpu_client_list, (void*)_match_cpu_fd);
}



void enviarAEjecutar(int32_t socketCPU, int32_t  quantum, t_process* aProcess){

		int32_t v1 = aProcess->tcb->pid;
		int32_t v2 = aProcess->tcb->program_counter;



		t_contenido mensaje;
		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d, %d, %d]", v1, v2,  config_kernel.QUANTUM));
		enviarMensaje(socketCPU, KERNEL_TO_CPU_TCB, mensaje, logKernel);
		log_info(logKernel, "Se envía un PCB al CPU libre elegido");

}

t_tcb* getProcesoDesdeCodigoBESO(char* stringCode, int32_t PID, int32_t TID, int32_t fd){

	t_tcb* process_tcb = malloc(sizeof(t_tcb));

	memset(process_tcb, 0, sizeof(process_tcb));

	process_tcb->pid = PID;
	process_tcb->tid = TID;
	process_tcb->indicador_modo_kernel = false;

	process_tcb->base_stack = solicitarSegmentoStack();
	process_tcb->base_segmento_codigo = solicitarSegmentoCodigo();

	return process_tcb;
}

int32_t solicitarSegmentoStack(){
	return 0;
}

int32_t solicitarSegmentoCodigo(){
	return 0;
}

