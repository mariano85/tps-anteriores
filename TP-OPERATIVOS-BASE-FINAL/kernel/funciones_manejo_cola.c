/*
 * funciones_manejo_cola.c
 *
 *  Created on: 16/10/2014
 *      Author: utnso
 */

#include "kernel.h"


bool stillInside(int32_t processFd){

	bool _match_fd(void* element) {
		if (((t_process*)element)->process_fd == processFd) {
			return true;
		}
		return false;
	}

	t_process* aProcess;
	bool response = false;

	pthread_mutex_lock(&mutex_ready_queue);
	if((aProcess = (t_process*)list_find(READY->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_ready_queue);
		response = true;
	}
	else{
		pthread_mutex_unlock(&mutex_ready_queue);
	}

	pthread_mutex_lock(&mutex_block_queue);
	if((aProcess = (t_process*)list_find(BLOCK->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_block_queue);
		response = true;
	}
	else{
		pthread_mutex_unlock(&mutex_block_queue);
	}

	pthread_mutex_lock(&mutex_exec_queue);
	if((aProcess = (t_process*)list_find(EXEC->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_exec_queue);
		response = true;
	}
	else{
		pthread_mutex_unlock(&mutex_exec_queue);
	}

	//pthread_mutex_lock(&mutex_exit_queue);
	if((aProcess = (t_process*)list_find(EXIT->elements, (void*)_match_fd)) != NULL){
		//pthread_mutex_unlock(&mutex_exit_queue);
		response = true;
	}
	else{
		//pthread_mutex_unlock(&mutex_exit_queue);
	}

	return response;

}


int32_t encontrarProcesoPorFD(int32_t fd){

	bool _match_fd(void* element) {
		if (((t_process*)element)->process_fd == fd) {
			return true;
		}
		return false;
	}

	/*Busco en qué cola está!*/
	t_process* aProcess;

	pthread_mutex_lock(&mutex_ready_queue);
	if((aProcess = (t_process*)list_find(READY->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_ready_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_ready_queue);
	}

	pthread_mutex_lock(&mutex_block_queue);
	if((aProcess = (t_process*)list_find(BLOCK->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_block_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_block_queue);
	}

	pthread_mutex_lock(&mutex_exec_queue);
	if((aProcess = (t_process*)list_find(EXEC->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_exec_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_exec_queue);
	}

	pthread_mutex_lock(&mutex_exit_queue);
	if((aProcess = (t_process*)list_find(EXIT->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_exit_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_exit_queue);
	}

	return 0;
}


int32_t encontrarProcesoPorPIDyTID(int32_t pid, int32_t tid){

	bool _match_pid_tid(void* element) {
		if ((((t_process*)element)->tcb->pid == pid) && (((t_process*)element)->tcb->tid == tid)){
			return true;
		}
		return false;
	}

	/*Busco en qué cola está!*/
	t_process* aProcess;

	pthread_mutex_lock(&mutex_ready_queue);
	if((aProcess = (t_process*)list_find(READY->elements, (void*)_match_pid_tid)) != NULL){
		pthread_mutex_unlock(&mutex_ready_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_ready_queue);
	}

	pthread_mutex_lock(&mutex_block_queue);
	if((aProcess = (t_process*)list_find(BLOCK->elements, (void*)_match_pid_tid)) != NULL){
		pthread_mutex_unlock(&mutex_block_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_block_queue);
	}

	pthread_mutex_lock(&mutex_exec_queue);
	if((aProcess = (t_process*)list_find(EXEC->elements, (void*)_match_pid_tid)) != NULL){
		pthread_mutex_unlock(&mutex_exec_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_exec_queue);
	}

	pthread_mutex_lock(&mutex_exit_queue);
	if((aProcess = (t_process*)list_find(EXIT->elements, (void*)_match_pid_tid)) != NULL){
		pthread_mutex_unlock(&mutex_exit_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_exit_queue);
	}

	return 0;
}


void agregarProcesoColaNew(t_process* aProcess) {

	bool estabaVacio = queue_size(READY) == 0;

	pthread_mutex_lock (&mutex_ready_queue);
	queue_push(READY, aProcess);
	pthread_mutex_unlock (&mutex_ready_queue);
	log_info(logKernel, "Un nuevo proceso se insertó en la cola NEW!");

	if(estabaVacio){
		pthread_cond_signal(&cond_ready_producer);
	}

}


void agregarProcesoKernel(t_process* aProcess) {

	/*
	 * lo pongo primero en la lista
	 */
	pthread_mutex_lock (&mutex_block_queue);
	list_add_in_index(BLOCK->elements, 0, aProcess);
	pthread_mutex_unlock (&mutex_block_queue);
	log_info(logKernel, "Se inserta el proceso de Kernel en la cola BLOCK!");

}


// aca viene la joda loca, hay que migrar todo tal cual. despues probar y ajustar
void agregarProcesoColaReady(t_process* aProcess) {


}


void agregarProcesoColaExec(){
	// la funcion va a ser igual que en el tp del cuatri anterior
	// porque cuando se desbloquee el tcb del kernel va a ir primero en la cola
	// asi que la planificacion seria como siempre
	t_client_cpu* aCpu;
	t_process* aProcess;

	// me fijo si hay una cpu libre
	log_info(logKernel, "antes de ver la cpu list");

	pthread_mutex_lock(&mutex_cpu_list);
		if(list_any_satisfy(cpu_client_list, (void*)cpuLibre)){
			aCpu = list_find(cpu_client_list, (void*)cpuLibre);
		}
	pthread_mutex_unlock(&mutex_cpu_list);

	if(aCpu != NULL){

		// saco un proceso de la cola de ready
		pthread_mutex_lock (&mutex_ready_queue);
			aProcess = queue_pop(READY);
		pthread_mutex_unlock (&mutex_ready_queue);

		if(aProcess == NULL){

			log_info(logKernel, "Se intentó poner en ejecución un nuevo programa, pero no había ninguno en READY. :/");
			log_info(logKernel,"Busco en la cola de NEW y lo nuevo a READY");

			pthread_mutex_lock (&mutex_ready_queue);
				aProcess = queue_pop(READY);

			pthread_mutex_unlock (&mutex_ready_queue);

		}
	pthread_mutex_lock (&mutex_exec_queue);
			queue_push(EXEC, aProcess);
		pthread_mutex_unlock (&mutex_exec_queue);

		aCpu->ocupado = true;
		aCpu->socketProceso = aProcess->process_fd;
		aCpu->pidTCB = aProcess->tcb->pid;
		aCpu->tidTCB = aProcess->tcb->tid;

		log_info(logKernel, string_from_format("Un nuevo programa entra en ejecución (PID: %d) (TID: %d) en Procesador PID: %d"
				, aProcess->tcb->pid
				, aProcess->tcb->tid
				, aCpu->cpuPID));

		enviarAEjecutar(aCpu->cpuFD, config_kernel.QUANTUM, aProcess);

	}

}


void enviarAEjecutar(int32_t socketCPU, int32_t  quantum, t_process* aProcess){

	int32_t v0 = aProcess->tcb->pid;
	int32_t v1 = aProcess->tcb->base_segmento_codigo;
	int32_t v2 = aProcess->tcb->tamanio_segmento_codigo;
	int32_t v3 = config_kernel.QUANTUM;
	int32_t v4 = aProcess->tcb->indicador_modo_kernel;
	int32_t v5 = aProcess->tcb->base_stack;
	int32_t v6 = aProcess->tcb->cursor_stack;

	// esto por ahora lo no recibe Nico, pero lo tiene que recibir
	int32_t v8 = aProcess->tcb->tid;

	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("[%d, %d, %d, %d, %d, %d, %d]", v0, v1, v2, v3, v4, v5, v6));
	enviarMensaje(socketCPU, KERNEL_TO_CPU_TCB, mensaje, logKernel);
	log_info(logKernel, "Se envía un TCB al CPU libre elegido");

}


// TODO revisar funcion
void agregarProcesoColaBlock(int32_t processFd, char* semaphoreKey){

	t_process* aProcess;
	bool _match_fd(void* element) {
		if (((t_process*)element)->process_fd == processFd) {
			return true;
		}
		return false;
	}

	/*Busco el proceso que se bloquea...obviamente tiene que estar en ejecucion!*/
	pthread_mutex_lock(&mutex_exec_queue);
		if((aProcess = (t_process*)list_find(EXEC->elements, (void*)_match_fd)) != NULL){
			t_list* aux_exec_list = EXEC->elements;
			list_remove_by_condition(aux_exec_list, _match_fd);
		}
	pthread_mutex_unlock(&mutex_exec_queue);

	/*Guardo el valor del semaforo en la estructura para saber por que se bloquea*/
	strcpy(aProcess->blockedBySemaphore, semaphoreKey);





	{
		queue_push(BLOCK, aProcess);
		log_info(logKernel, string_from_format("El proceso PID %d se encuentra bloqueado por semaforo: %s", aProcess->tcb->pid, semaphoreKey));
		pthread_mutex_unlock(&mutex_block_queue);/* release the buffer */
	}

}


void agregarProcesoColaExit(t_process* aProcess){

	pthread_mutex_lock(&mutex_exit_queue);	/* protect buffer */
	while (EXIT->elements->elements_count != 0){/* If there is something in the buffer then wait */
	  pthread_cond_wait(&cond_exit_producer, &mutex_exit_queue);
	}
	  //Makes things happen here! Inserts a new finished process into exit queue!
		log_debug(logKernel, "Un nuevo proceso (PID: %i) ha finalizado. Se inserta en la cola de EXIT :", aProcess->tcb->pid);
		queue_push(EXIT, aProcess);

	pthread_cond_signal(&cond_exit_consumer);	/* wake up consumer */
	pthread_mutex_unlock(&mutex_exit_queue);	/* release the buffer */

}


void removeProcess(int32_t processPID, bool someoneKilledHim){

	bool _match_process_pid(void* element) {
		if (((t_process*)element)->tcb->pid== processPID) {
			return true;
		}
		return false;
	}

	pthread_mutex_lock (&mutex_block_queue);
	pthread_mutex_lock (&mutex_ready_queue);
	pthread_mutex_lock (&mutex_exec_queue);
	bool wasNew = false;

	/*Busco en qué cola está!*/
	t_process* aProcess;
	bool Hit = false; //Verifica si no se mato el proceso mientras lo buscaba

	if((aProcess = (t_process*)list_find(READY->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(READY->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(BLOCK->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(BLOCK->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(EXEC->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(EXEC->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(EXIT->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(EXIT->elements, (void*)_match_process_pid);
	}
	pthread_mutex_unlock (&mutex_exec_queue);
	pthread_mutex_unlock (&mutex_ready_queue);
	pthread_mutex_unlock (&mutex_block_queue);

	if(Hit){
		if(!someoneKilledHim){/*Si nadie lo mató, entonces entró en la cola de EXIT y el PLP lo está expulsando del sistema*/
			//ComunicarMuertePrograma(aProcess->pid);
		}

		killProcess(aProcess);
	}
}


// esto es un hilo
void manejo_cola_ready(void){

	int myTid = process_get_thread_id();
	log_info(logKernel, "**************Manejo cola ready (PID: %d) ***************",myTid);
	log_info(logKernel, "**************Paso a ready los procesos de la cola new****",myTid);
	bool cpuLibre = false;

	while(true){
//		pthread_mutex_lock(&mutex_ready_queue);	/* protect buffer */

		while(READY->elements->elements_count == 0 ||
			(READY->elements->elements_count > 0 && !list_any_satisfy(cpu_client_list, (void*)cpuLibre)) ) {

			if(	(READY->elements->elements_count > 0 && !list_any_satisfy(cpu_client_list, (void*)cpuLibre)) ) {
				log_info(logKernel, "Hilo Manejador de la cola READY: Un nuevo proceso listo! Pero ninguna CPU conectada o libre aún! :( No hago nada hasta que no levanten una!");
			}

			pthread_cond_wait(&cond_ready_consumer, &mutex_ready_queue);
		}

	}
	//	pthread_mutex_unlock(&mutex_ready_queue);

	log_info(logKernel, "Queue Manager Thread Says: Un nuevo proceso listo! Voy a pasarlo a ejecución!");
	agregarProcesoColaExec();

	pthread_cond_signal(&cond_ready_producer);	/* wake up producer */

}


// esto es otro hilo
void manejo_cola_exit(void){

	int myTid = process_get_thread_id();
	log_info(logKernel, "************** Exit Manager Thread Started (TID: %d) ***************",myTid);

	for(;;){

		pthread_mutex_lock(&mutex_exit_queue);	/* protect buffer */
		while (EXIT->elements->elements_count == 0)/* If there is nothing in the buffer then wait */
		{
			pthread_cond_wait(&cond_exit_consumer, &mutex_exit_queue);
		}

		/*Make things happen here!*/
		log_debug(logKernel, "Exit Queue Manager Thread Says: Hey!I Got you dude!");
		int32_t counter = 0;
		for(counter = 0; counter < EXIT->elements->elements_count; counter ++ ){
			t_tcb* aProcess = (t_tcb*)queue_peek(EXIT);

			removeProcess(aProcess->pid, false);
		}
		pthread_cond_signal(&cond_exit_producer);	/* wake up producer */
		pthread_mutex_unlock(&mutex_exit_queue);	/* release the buffer */

	}

}


bool cpuLibre(void* element){
	return ((t_client_cpu*)element)->ocupado == false;
}

t_client_cpu* buscarCpuPorSocket(int32_t cpuFd){

	bool _match_cpu_fd(void* element){
		return ((t_client_cpu*)element)->cpuFD == cpuFd;
	}

	return list_find(cpu_client_list, (void*)_match_cpu_fd);
}
