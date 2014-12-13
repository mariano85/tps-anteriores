/*
 * funciones_manejo_cola.c
 *
 *  Created on: 16/10/2014
 *      Author: utnso
 */

#include "kernel.h"


void setearProcesoCola(t_process* aProcess, t_cola cola){

	if(aProcess->tcb->kernel_mode){
		pthread_mutex_lock(&mutex_tcb_km);
		aProcess->tcb->cola = cola;
		pthread_mutex_unlock(&mutex_tcb_km);
	} else {
		aProcess->tcb->cola = cola;
	}

}


void agregarProcesoColaNew(t_process* aProcess) {

	setearProcesoCola(aProcess, NEW);
	pushColaReady(aProcess);

}


void agregarProcesoColaReady(t_process* aProcess) {

	setearProcesoCola(aProcess, READY);
	pushColaReady(aProcess);

}


void pushColaReady(t_process* aProcess) {
	pthread_mutex_lock(&mutex_ready_queue);	/* Blocks the buffer */
		while ((COLA_READY->elements->elements_count != 0) && list_any_satisfy(cpu_client_list, (void*)cpuLibre)){
	//				log_info(kernelLog, "READY PUSH");
			pthread_cond_wait(&cond_ready_producer, &mutex_ready_queue);
		}
	/*Magic here, please!*/

		if(aProcess->tcb->kernel_mode){
			// lo inserto al principio de la cola porque es el tcb km
			list_add_in_index(COLA_READY->elements, 0, aProcess);
		} else {
			queue_push(COLA_READY, aProcess);
		}

		pthread_cond_signal(&cond_ready_consumer);			/* wake up consumer */
	pthread_mutex_unlock(&mutex_ready_queue);	/* release the buffer */

}


// esto es un hilo. Y TIENE QUE ANDAR
void manejo_cola_ready() {

	int myPid = process_get_thread_id();
	log_info(logKernel, "**************Ready Queue Manager Thread Started (PID: %d) ***************",myPid);

	for(;;){
		//printf("pase por el queue manageeeeer");
		pthread_mutex_lock(&mutex_ready_queue);	/* protect buffer */
		while (COLA_READY->elements->elements_count == 0 ||
					(!list_any_satisfy(cpu_client_list, (void*)cpuLibre)
						&& COLA_READY->elements->elements_count != 0))/* If there is nothing in the buffer then wait */
		{
			if(!list_any_satisfy(cpu_client_list, (void*)cpuLibre)
					&& COLA_READY->elements->elements_count != 0){
			log_info(logKernel, "Queue Manager Thread Says: Un nuevo proceso listo! Pero ninguna CPU conectada o libre aún! :( No hago nada hasta que no levanten una!");
			}

			pthread_cond_wait(&cond_ready_consumer, &mutex_ready_queue);
		}

		pthread_mutex_unlock(&mutex_ready_queue);
			log_info(logKernel, "Queue Manager Thread Says: Un nuevo proceso listo! Voy a pasarlo a ejecución!");
			agregarProcesoColaExec();

		pthread_cond_signal(&cond_ready_producer);	/* wake up producer */
			/* release the buffer */
	}
}


void enviarAEjecutar(int32_t socketCPU, int32_t  quantum, t_process* aProcess) {

	int32_t v1 = aProcess->tcb->pid;
	int32_t v2 = aProcess->tcb->segmento_codigo;
	int32_t v3 = aProcess->tcb->segmento_codigo_size;
	int32_t v4 = config_kernel.QUANTUM;
	int32_t v5 = aProcess->tcb->kernel_mode;
	int32_t v6 = aProcess->tcb->base_stack;
	int32_t v7 = aProcess->tcb->cursor_stack;

	// esto por ahora lo no recibe Nico, pero lo tiene que recibir
	int32_t v8 = aProcess->tcb->tid;

	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("[%d, %d, %d, %d, %d, %d, %d, %d]", v1, v2, v3, v4, v5, v6, v7, v8));
	enviarMensaje(socketCPU, KERNEL_TO_CPU_TCB, mensaje, logKernel);
	log_info(logKernel, "Se envía un TCB al CPU libre elegido");

}


void agregarProcesoColaExec(){

	t_client_cpu* aCpu;
	t_process* aProcess;

	pthread_mutex_lock(&mutex_cpu_list);
	//	log_debug(kernelLog, "BUSCO CPU LIBRE PARA EJECUTAR PROGRAMA!");
		if(list_any_satisfy(cpu_client_list, (void*)cpuLibre)){
			aCpu = list_find(cpu_client_list, (void*)cpuLibre);
		}
		pthread_mutex_lock (&mutex_ready_queue);
		// si la cola está ready
	//		log_debug(kernelLog, "SACO PROGRAMA DE READY!");
			aProcess = queue_pop(COLA_READY);
		pthread_mutex_unlock (&mutex_ready_queue);

	pthread_mutex_unlock(&mutex_cpu_list);

	if(aCpu != NULL){

		if(aProcess != NULL){
//			log_debug(kernelLog, "HABIA UN PROGRAMA EN READY!");
			// esta es la magia: no hay cola exec. simplemente lo asocio a la cpu encontrada
			if(aProcess->tcb->cola == NEW){
				setearProcesoCola(aProcess, READY);
				log_info(logKernel, string_from_format("El proceso (PID: %d) era Nuevo y está Listo para ejecutarse en Procesador PID: %d", aProcess->tcb->pid, aCpu->cpuPID));
			}

			setearProcesoCola(aProcess, EXEC);
			aCpu->procesoExec = aProcess;

			log_info(logKernel, string_from_format("Procede a ejecutarse el proceso (PID: %d) en Procesador PID: %d", aProcess->tcb->pid, aCpu->cpuPID));
			enviarAEjecutar(aCpu->cpuFD, config_kernel.QUANTUM, aProcess);

		} else {
			log_info(logKernel, "Se intentó poner en ejecución un nuevo programa, pero no había ninguno en READY. :/");
			log_info(logKernel, string_from_format("CPU (PID: %d) aguarda instrucciones!", aCpu->cpuPID));
		}

	} else {
		log_info(logKernel, string_from_format("Se intentó poner en ejecución un nuevo programa, pero no había ningun CPU libre o activo! :/"));
	}

}


// TODO HACER funcion
void agregarProcesoColaSyscall(t_process* unProceso, uint32_t direccion){
	// aca seteo directamente porque se que aca no va a entrar el proceso km
	unProceso->tcb->cola = BLOCK;
	unProceso->direccion_syscall = direccion;

	pthread_mutex_lock(&mutex_syscalls_queue);	/* protect buffer */
	queue_push(COLA_SYSCALLS, unProceso);
	pthread_mutex_unlock(&mutex_syscalls_queue);	/* protect buffer */
}

/*
 * esta funcion TIENE que funcionar, junto con el hilo
 */
void agregarProcesoColaExit(t_process* aProcess){

	// aca seteo directamente porque se que aca no va a entrar el proceso km
	aProcess->tcb->cola = EXIT;

	pthread_mutex_lock(&mutex_exit_queue);	/* protect buffer */
	while (COLA_EXIT->elements->elements_count != 0){/* If there is something in the buffer then wait */
	  pthread_cond_wait(&cond_exit_producer, &mutex_exit_queue);
	}
	  //Makes things happen here! Inserts a new finished process into exit queue!
		log_debug(logKernel, "Un nuevo proceso (PID: %i) ha finalizado. Se inserta en la cola de EXIT :", aProcess->tcb->pid);
		queue_push(COLA_EXIT, aProcess);

	pthread_cond_signal(&cond_exit_consumer);	/* wake up consumer */
	pthread_mutex_unlock(&mutex_exit_queue);	/* release the buffer */

}


// esto es otro hilo
void manejo_cola_exit() {

	int myTid = process_get_thread_id();
	log_info(logKernel, "************** Exit Manager Thread Started (TID: %d) ***************",myTid);

	for(;;){

		pthread_mutex_lock(&mutex_exit_queue);	/* protect buffer */
		while (COLA_EXIT->elements->elements_count == 0)/* If there is nothing in the buffer then wait */
		{
			pthread_cond_wait(&cond_exit_consumer, &mutex_exit_queue);
		}

		/*Make things happen here!*/
		log_debug(logKernel, "Exit Queue Manager Thread Says: Hey!I Got you dude!");
		int32_t counter = 0;
		for(counter = 0; counter < COLA_EXIT->elements->elements_count; counter ++ ){
			t_process* aProcess = (t_process*)queue_peek(COLA_EXIT);

			removeProcess(aProcess->tcb->pid, false);
		}
		pthread_cond_signal(&cond_exit_producer);	/* wake up producer */
		pthread_mutex_unlock(&mutex_exit_queue);	/* release the buffer */

	}

}


void removeProcess(int32_t processPID, bool someoneKilledHim){

	bool _match_process_pid(void* element) {
		if (((t_process*)element)->tcb->pid== processPID) {
			return true;
		}
		return false;
	}

	pthread_mutex_lock (&mutex_syscalls_queue);
	pthread_mutex_lock (&mutex_ready_queue);
	pthread_mutex_lock (&mutex_exec_queue);

	/*Busco en qué cola está!*/
	t_process* aProcess;
	bool Hit = false; //Verifica si no se mato el proceso mientras lo buscaba

	if((aProcess = (t_process*)list_find(COLA_READY->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(COLA_READY->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(COLA_SYSCALLS->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(COLA_SYSCALLS->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(COLA_EXIT->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(COLA_EXIT->elements, (void*)_match_process_pid);
	}
	pthread_mutex_unlock (&mutex_exec_queue);
	pthread_mutex_unlock (&mutex_ready_queue);
	pthread_mutex_unlock (&mutex_syscalls_queue);

	if(Hit){
		if(!someoneKilledHim){/*Si nadie lo mató, entonces entró en la cola de EXIT y el PLP lo está expulsando del sistema*/
			//ComunicarMuertePrograma(aProcess->pid);
		}

		killProcess(aProcess);
	}
}


bool cpuLibre(void* element){
	return ((t_client_cpu*)element)->procesoExec == NULL;
}


bool cpuEjecutaTCBKM(void *element){
	return ((t_client_cpu*)element)->procesoExec != NULL && ((t_client_cpu*)element)->procesoExec->tcb->kernel_mode;
}


/*
 * TODO: implementarla
 */
bool stillInside(int32_t processFd){

	return true;

}


/*
 * TODO: implementarla
 */
int32_t encontrarProcesoPorFD(int32_t fd) {

	return 0;
}


int32_t encontrarProcesoPorPIDyTID(int32_t pid, int32_t tid) {

	bool _match_pid_tid(void* element) {
		if ((((t_process*)element)->tcb->pid == pid) && (((t_process*)element)->tcb->tid == tid)){
			return true;
		}
		return false;
	}

	/*Busco en qué cola está!*/
	t_process* aProcess;

	pthread_mutex_lock(&mutex_ready_queue);
	if((aProcess = (t_process*)list_find(COLA_READY->elements, (void*)_match_pid_tid)) != NULL){
		pthread_mutex_unlock(&mutex_ready_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_ready_queue);
	}

	pthread_mutex_lock(&mutex_syscalls_queue);
	if((aProcess = (t_process*)list_find(COLA_SYSCALLS->elements, (void*)_match_pid_tid)) != NULL){
		pthread_mutex_unlock(&mutex_syscalls_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_syscalls_queue);
	}

	pthread_mutex_lock(&mutex_exit_queue);
	if((aProcess = (t_process*)list_find(COLA_EXIT->elements, (void*)_match_pid_tid)) != NULL){
		pthread_mutex_unlock(&mutex_exit_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_exit_queue);
	}

	return 0;
}
