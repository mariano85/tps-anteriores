/*
 * funciones_manejo_cola.c
 *
 *  Created on: 16/10/2014
 *      Author: utnso
 */

#include "kernel.h"

t_log* logKernel;
t_log* queueLog;

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
		if((aProcess = (t_process*)list_find(ready_queue->elements, (void*)_match_fd)) != NULL){
			pthread_mutex_unlock(&mutex_ready_queue);
			response = true;
		}
		else{
			pthread_mutex_unlock(&mutex_ready_queue);
		}

		pthread_mutex_lock(&mutex_new_queue);
		if((aProcess = (t_process*)list_find(new_queue->elements, (void*)_match_fd)) != NULL){
			pthread_mutex_unlock(&mutex_new_queue);
			response = true;
		}
		else{
			pthread_mutex_unlock(&mutex_new_queue);
		}

		pthread_mutex_lock(&mutex_block_queue);
		if((aProcess = (t_process*)list_find(block_queue->elements, (void*)_match_fd)) != NULL){
			pthread_mutex_unlock(&mutex_block_queue);
			response = true;
		}
		else{
			pthread_mutex_unlock(&mutex_block_queue);
		}

		pthread_mutex_lock(&mutex_exec_queue);
		if((aProcess = (t_process*)list_find(exec_queue->elements, (void*)_match_fd)) != NULL){
			pthread_mutex_unlock(&mutex_exec_queue);
			response = true;
		}
		else{
			pthread_mutex_unlock(&mutex_exec_queue);
		}

		//pthread_mutex_lock(&mutex_exit_queue);
		if((aProcess = (t_process*)list_find(exit_queue->elements, (void*)_match_fd)) != NULL){
			//pthread_mutex_unlock(&mutex_exit_queue);
			response = true;
		}
		else{
			//pthread_mutex_unlock(&mutex_exit_queue);
		}

		return response;

}

int32_t getProcessPidByFd(int32_t fd){

	bool _match_fd(void* element) {
		if (((t_process*)element)->process_fd == fd) {
			return true;
		}
		return false;
	}

	/*Busco en qué cola está!*/
	t_process* aProcess;

	pthread_mutex_lock(&mutex_ready_queue);
	if((aProcess = (t_process*)list_find(ready_queue->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_ready_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_ready_queue);
	}


	pthread_mutex_lock(&mutex_new_queue);
	if((aProcess = (t_process*)list_find(new_queue->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_new_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_new_queue);
	}

	pthread_mutex_lock(&mutex_block_queue);
	if((aProcess = (t_process*)list_find(block_queue->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_block_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_block_queue);
	}

	pthread_mutex_lock(&mutex_exec_queue);
	if((aProcess = (t_process*)list_find(exec_queue->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_exec_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_exec_queue);
	}

	pthread_mutex_lock(&mutex_exit_queue);
	if((aProcess = (t_process*)list_find(exit_queue->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_exit_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_exit_queue);
	}

	return 0;
}

/**
 * @NAME: agregar nuevo proceso
 * @DESC: Agrega un proceso la lista compartida new_queue
 * usando semaforo mutex.
 */
void addNewProcess(t_process* aProcess) {
	int32_t SystemProcessCounter = 0;

	// por ahora directamente la agregamos a la cola new

//	SystemProcessCounter = ObtenerCantidadDeProcesosEnSistema();
//	if(config_kernel.MULTIPROG > SystemProcessCounter){
//		addReadyProcess(aProcess);
//	}
//	else{
		pthread_mutex_lock (&mutex_new_queue);
		queue_push(new_queue, aProcess);
		pthread_mutex_unlock (&mutex_new_queue);
		log_info(logKernel, "Se intentó encolar un nuevo proceso listo(Ready), pero el Sistema alcanzó el máximo de programas según grado de multiprogramación.");
		log_info(logKernel, "Un nuevo proceso se insertó en la cola de listos!");
		printQueues();
//	}
}


/*Remove process by File Descriptor - Full Scann*/
void removeProcess(int32_t processPID, bool someoneKilledHim){

	bool _match_process_pid(void* element) {
		if (((t_process*)element)->tcb->pid == processPID) {
			return true;
		}
		return false;
	}

	pthread_mutex_lock (&mutex_block_queue);
	pthread_mutex_lock (&mutex_ready_queue);

	/*Ojo con DESBLOQUEAR ESTO! YA SE PIDIÓ ESTE RECURSO EN EL QUEUE_MANAGER AL INVOCAR ESTA FUNCION*/
	//pthread_mutex_lock (&mutex_exit_queue);
	pthread_mutex_lock (&mutex_exec_queue);
	pthread_mutex_lock (&mutex_new_queue);
	bool wasNew = false;



	/*Busco en qué cola está!*/
	t_process* aProcess;
	bool Hit = false; //Verifica si no se mato el proceso mientras lo buscaba

	if((aProcess = (t_process*)list_find(ready_queue->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(ready_queue->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(new_queue->elements, (void*)_match_process_pid)) != NULL){
		wasNew = true;
		Hit = true;
		list_remove_by_condition(new_queue->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(block_queue->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(block_queue->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(exec_queue->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(exec_queue->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(exit_queue->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(exit_queue->elements, (void*)_match_process_pid);
	}
	pthread_mutex_unlock (&mutex_new_queue);
	pthread_mutex_unlock (&mutex_exec_queue);
	//pthread_mutex_unlock (&mutex_exit_queue);
	pthread_mutex_unlock (&mutex_ready_queue);
	pthread_mutex_unlock (&mutex_block_queue);

	if(Hit){
		if(!someoneKilledHim){/*Si nadie lo mató, entonces entró en la cola de EXIT y el PLP lo está expulsando del sistema*/
			comunicarMuertePrograma(aProcess->process_fd, aProcess->in_umv);
		}
		/*Si el proceso que murió no era uno nuevo, entonces veo si por el gr. de multiprog puedo meter uno nuevo!*/
		if(!wasNew){
			checkNewProcesses();
		}
		/*Si estaba en memoria borro los segmentos*/
		if(aProcess->in_umv){
			eliminarSegmentos(processPID);
		}

		killProcess(aProcess);
	}
}

/*
 * @NAME: PrintReadyQueue
 * @DESC: Imprime el listado actual de elementos en la cola de READY
 *
 */
void printQueues(){

	log_info(queueLog, "NEW QUEUE -");
	printQueue(new_queue, mutex_new_queue, queueLog);
	log_info(queueLog, " ");
	log_info(queueLog, "READY QUEUE -");
	printQueue(ready_queue, mutex_ready_queue, queueLog);
	log_info(queueLog, " ");
	log_info(queueLog, "EXEC QUEUE -");
	printQueue(exec_queue, mutex_exec_queue, queueLog);
	log_info(queueLog, " ");
	log_info(queueLog, "BLOCK QUEUE -");
	printQueue(block_queue, mutex_block_queue, queueLog);
	log_info(queueLog, " ");
	log_info(queueLog, "EXIT QUEUE -");
	printQueue(exit_queue, mutex_exit_queue, queueLog);
	log_info(queueLog, " ");

}

/*
 *
 * Prints a queue, using mutex and a logger
 *
 */
void printQueue(t_queue* aQueue, pthread_mutex_t queueMutex, t_log* logger){

	t_list* list_aux = list_create();

//	pthread_mutex_lock (&queueMutex);
	list_add_all(list_aux, aQueue->elements);
//	pthread_mutex_unlock (&queueMutex);

	int counter;

	log_info(logger, "********************Process Status Report********************");
	if(list_size(list_aux) > 0){
		log_info(logger, "-----------------PID---------FD---------------");
		for(counter = 0; counter < list_size(list_aux); counter++){
			t_process* aProcess = (t_process*)list_get(list_aux, counter);
			log_info(logger, "-----------------%d-----------%d------------", aProcess->tcb->pid, aProcess->process_fd);
		}
		log_info(logger, "*************************************************************");
	}
	else{
		log_info(logger, "There is any process here yet!\n");
	}
}

void checkNewProcesses(){
	log_info(queueLog, "implementar checkNewProcesses");
}

t_process* getProcessStructureByBESOCode(char* code, int32_t pid, int32_t fd){
	log_info(queueLog, "implementar checkNewProcesses");
	return NULL;
}
