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


void enviarAEjecutar(int32_t socketCPU, t_process* aProcess) {

	int32_t v0 = aProcess->tcb->pid;
	int32_t v1 = aProcess->tcb->tid;
	int32_t v2 = aProcess->tcb->kernel_mode;
	int32_t v3 = aProcess->tcb->segmento_codigo;
	int32_t v4 = aProcess->tcb->segmento_codigo_size;
	int32_t v5 = aProcess->tcb->puntero_instruccion;
	int32_t v6 = aProcess->tcb->base_stack;
	int32_t v7 = aProcess->tcb->cursor_stack;
	int32_t v8 = aProcess->tcb->registros[0];
	int32_t v9 = aProcess->tcb->registros[1];
	int32_t v10 = aProcess->tcb->registros[2];
	int32_t v11 = aProcess->tcb->registros[3];
	int32_t v12 = aProcess->tcb->registros[4];

	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("[%d, %d, %d, %d, %d, %d, %d, %d,%d,%d,%d,%d,%d]", v0, v1, v2, v3, v4, v5, v6, v7,v8,v9,v10,v11,v12));
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
			enviarAEjecutar(aCpu->cpuFD, aProcess);

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
void agregarProcesoColaExit(t_process* aProcess, t_cola condicionDeSalida){

	// aca seteo directamente porque se que aca no va a entrar el proceso km
	aProcess->tcb->cola = condicionDeSalida;

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
			t_process* aProcess = (t_process*)queue_pop(COLA_EXIT);

			liberarProceso(aProcess);
		}
		pthread_cond_signal(&cond_exit_producer);	/* wake up producer */
		pthread_mutex_unlock(&mutex_exit_queue);	/* release the buffer */

	}

}


void liberarProceso(t_process* proceso){

	// notificar al padre que terminó
	notificarAlPadre(proceso);

	// elimino segmentos
	eliminarSegmento(proceso->tcb->pid, proceso->tcb->base_stack);

	// si el tcb es padre, elimino tambien el segmento de codigo
	if(proceso->tcb->tid == 0){
		eliminarSegmento(proceso->tcb->pid, proceso->tcb->segmento_codigo);
		notificarALosHijos(proceso);
	}

	/*
	 * en este switch veo qué mensaje le mando a la consola para que termine
	 */
	switch((char)proceso->tcb->cola){
	case EXIT:
		break;
	case EXIT_ERROR:
		break;
	case EXIT_ABORT_CPU:
		break;
	case EXIT_ABORT_CON:
		break;
	}

	// libero el proceso en si!
	free(proceso->tcb);
	free(proceso);

}


void notificarAlPadre(t_process* proceso){

}


void notificarALosHijos(t_process* proceso){

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


/*
 * TODO: faltan las colas de bloqueados por recursos...
 */
t_process* encontrarYRemoverProcesoPorFD(int32_t fd) {

	/*Busco en qué cola está!*/
	t_process* aProcess = NULL;
	t_client_cpu* cpu = NULL;

	bool _match_fd(void* element) {
		return ((t_process*)element)->process_fd == fd;
	}

	bool _match_cpu_fd(void* element){
		cpu = (t_client_cpu*)element;
		return cpu == NULL ? false : cpu->procesoExec->process_fd == fd;
	}

	if(aProcess == NULL){
		pthread_mutex_lock (&mutex_syscalls_queue);
		aProcess = list_remove_by_condition(COLA_SYSCALLS->elements, (void*)_match_fd);
		pthread_mutex_unlock (&mutex_syscalls_queue);
	}

	if(aProcess == NULL){
		pthread_mutex_lock (&mutex_ready_queue);
		aProcess = list_remove_by_condition(COLA_READY->elements, (void*)_match_fd);
		pthread_mutex_unlock (&mutex_ready_queue);
	}

	if(aProcess == NULL){
		pthread_mutex_lock (&mutex_cpu_list);
		t_client_cpu* cpu = list_find(cpu_client_list, (void*)_match_cpu_fd);
		aProcess = cpu->procesoExec;
		cpu->procesoExec = NULL;
		pthread_mutex_unlock (&mutex_cpu_list);
	}

	return aProcess;
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
