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

		pthread_mutex_lock(&mutex_new_queue);
		if((aProcess = (t_process*)list_find(NEW->elements, (void*)_match_fd)) != NULL){
			pthread_mutex_unlock(&mutex_new_queue);
			response = true;
		}
		else{
			pthread_mutex_unlock(&mutex_new_queue);
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
	if((aProcess = (t_process*)list_find(READY->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_ready_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_ready_queue);
	}


	pthread_mutex_lock(&mutex_new_queue);
	if((aProcess = (t_process*)list_find(NEW->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_new_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_new_queue);
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

/**
 * @NAME: agregar nuevo proceso
 * @DESC: Agrega un proceso la lista compartida new_queue
 * usando semaforo mutex.
 */






/* FUNCIONES MANEJO COLA -------------------
 * ---------------------
 * -------------------------
 * ------------------
 */

void mostrarColas(){

		log_debug(queueLog, "NEW QUEUE -");
		mostrarCola(NEW, mutex_new_queue, queueLog);
		log_info(queueLog, " ");
		log_info(queueLog, "READY QUEUE -");
		mostrarCola(READY, mutex_ready_queue, queueLog);
		log_info(queueLog, " ");
		log_info(queueLog, "EXEC QUEUE -");
		mostrarCola(EXEC, mutex_exec_queue, queueLog);
		log_info(queueLog, " ");
		log_info(queueLog, "BLOCK QUEUE -");
		mostrarCola(BLOCK, mutex_block_queue, queueLog);
		log_info(queueLog, " ");
		log_info(queueLog, "EXIT QUEUE -");
		mostrarCola(EXIT, mutex_exit_queue, queueLog);
		log_info(queueLog, " ");



	}

	bool ordernRoundRobin(void* p1, void* p2){
		t_tcb* processA_aux = (t_tcb*)p1;
		t_tcb* processB_aux = (t_tcb*)p2;

		return processA_aux->pid < processB_aux->pid;
	}



	void mostrarCola(t_queue* aQueue, pthread_mutex_t queueMutex, t_log* logger){

		t_list* list_aux = list_create();


		list_add_all(list_aux, aQueue->elements);


		int counter;

		log_info(logger, "********************Process Status Report********************");
		if(list_size(list_aux) > 0){
			log_info(logger, "-----------------PID---------WEIGHT---------FD---------------");
			for(counter = 0; counter < list_size(list_aux); counter++){
				t_tcb* aProcess = (t_tcb*)list_get(list_aux, counter);
				log_info(logger, "-----------------%d-----------%d------------%d---------------", aProcess->pid, aProcess->tamanio_indice_codigo, aProcess->program_counter);
			}
			log_info(logger, "*************************************************************");
		}
		else{
			log_debug(logger, "No hay proesos :(!\n");
		}
	}





void agregarProcesoColaNew(t_process* aProcess) {


		pthread_mutex_lock (&mutex_new_queue);
		queue_push(NEW, aProcess);
		pthread_mutex_unlock (&mutex_new_queue);
		puts("Un nuevo proceso se insertó en la cola NEW!");
		log_info(logKernel, "Un nuevo proceso se insertó en la cola NEW!");








}

void agregarProcesoColaReady(t_process* aProcess) {
	t_process* process_aux;

	if(aProcess != NULL){
		process_aux = aProcess;
	}
	else{
		pthread_mutex_lock (&mutex_new_queue);
			/*Ordeno cola de NEW por algoritmo de SJN*/

			t_list* list_aux;
			list_aux =  NEW->elements;
	//		list_sort(list_aux, (void*)ordernRoundRobin);   ///////// VER EL ORDEN QUE VAMOS A USAR

			NEW->elements = list_aux;

			/*Extraigo el primero mas liviano*/
			process_aux = queue_pop(NEW);
			pthread_mutex_unlock (&mutex_new_queue);
	}
}



	void agregarProcesoColaExec(){

			t_client_cpu* aCpu;
			t_tcb* aProcess;
			bool disponible = false;

			pthread_mutex_lock(&mutexCPUDISP);
			log_debug(logKernel, "BUSCO CPU LIBRE PARA EJECUTAR PROGRAMA!");

			bool _cpuLibre(void* element){

					if(((t_client_cpu*)element)->ocupado == false){
						return true;
					}
					return false;
				}

			if(list_any_satisfy(cpu_disponibles_list, (void*)_cpuLibre)){
				aCpu = list_find(cpu_disponibles_list, (void*) _cpuLibre);
				disponible = true;
			}
			pthread_mutex_unlock(&mutexCPUDISP);


		if(disponible){
			pthread_mutex_lock (&mutex_ready_queue);

				aProcess = queue_pop(READY);
				pthread_mutex_unlock (&mutex_ready_queue);

			if(aProcess != NULL){

				pthread_mutex_lock (&mutex_exec_queue);
					queue_push(EXEC, aProcess);
					pthread_mutex_unlock (&mutex_exec_queue);



				aCpu->ocupado = true;
				aCpu->processPID = aProcess->pid;
				//log_debug(kernelLog,("Un nuevo programa entra en ejecución (PID: %d) en Procesador PID: %d", aProcess->pid, aCpu->cpuPID));

				//enviarAEjecutar(aCpu->cpuFD, config_kernel.QUANTUM, aProcess);

				mostrarColas();
			}
			else{
				log_info(logKernel, "Se intentó poner en ejecución un nuevo programa, pero no había ninguno en READY. :/");
				log_info(logKernel, string_from_format("CPU (PID: %d) aguarda instrucciones!", aCpu->cpuPID));
			}
		}
		else{
			log_info(logKernel, string_from_format("Se intentó poner en ejecución un nuevo programa, pero no había ningun CPU libre o activo! :/"));
		}
	}

	void agregarProcesoColaBlock(int32_t processFd, char* semaphoreKey, char* ioKey, int32_t io_tiempo){
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

		mostrarColas();



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


	void chequearProcesos(){
		//if(config_kernel.MULTIPROG > ObtenerCantidadDeProcesosEnSistema()){
			if(queue_size(NEW) > 0){
				agregarProcesoColaReady(NULL);
			}
			else{
				log_info(logKernel, "La cola de procesos nuevos Ready se encuentra llena..");
			}
		//}
	}

	int32_t cantDeProcesosEnSistema(){

			int32_t counter = 0;

			pthread_mutex_lock (&mutex_block_queue);
				pthread_mutex_lock (&mutex_ready_queue);
				pthread_mutex_lock (&mutex_exec_queue);
				counter += READY->elements->elements_count;
				counter += EXEC->elements->elements_count;
				counter += BLOCK->elements->elements_count;
				pthread_mutex_unlock (&mutex_exec_queue);
					pthread_mutex_unlock (&mutex_ready_queue);
					pthread_mutex_unlock (&mutex_block_queue);

			//log_debug(archivo_logs, ("Cantidad de procesos en el sistema: %d", counter));

			return counter;
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
	pthread_mutex_lock (&mutex_new_queue);
	bool wasNew = false;



	/*Busco en qué cola está!*/
	t_process* aProcess;
	bool Hit = false; //Verifica si no se mato el proceso mientras lo buscaba

	if((aProcess = (t_process*)list_find(READY->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(READY->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(NEW->elements, (void*)_match_process_pid)) != NULL){
		wasNew = true;
		Hit = true;
		list_remove_by_condition(NEW->elements, (void*)_match_process_pid);
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
	pthread_mutex_unlock (&mutex_new_queue);
	pthread_mutex_unlock (&mutex_exec_queue);
	pthread_mutex_unlock (&mutex_ready_queue);
	pthread_mutex_unlock (&mutex_block_queue);

	if(Hit){
		if(!someoneKilledHim){/*Si nadie lo mató, entonces entró en la cola de EXIT y el PLP lo está expulsando del sistema*/
			//ComunicarMuertePrograma(aProcess->pid);
		}
		/*Si el proceso que murió no era uno nuevo, entonces veo si por el gr. de multiprog puedo meter uno nuevo!*/
		if(!wasNew){
			chequearProcesos();
		}


		killProcess(aProcess);
	}
}



void manejo_cola_ready(void){

	int myPid = 1; // probando
	log_info(logKernel, "**************Ready Queue Manager Thread Started (PID: %d) ***************",myPid);
	bool hayCpuLibre = false;
	t_client_cpu* cpu;
	for(;;){
		hayCpuLibre = false;

		pthread_mutex_lock(&mutex_ready_queue);


		bool _cpuLibre(void* element){

						if(((t_client_cpu*)element)->ocupado == false){
							return true;
						}
						return false;
					}

		while (READY->elements->elements_count == 0 ||

					(!list_any_satisfy(cpu_disponibles_list, (void*)_cpuLibre)
						&& READY->elements->elements_count != 0))/* If there is nothing in the buffer then wait */
		{
			if(!list_any_satisfy(cpu_disponibles_list, (void*)_cpuLibre)
					&& READY->elements->elements_count != 0){
			log_info(logKernel, "Queue Manager Thread Says: Un nuevo proceso listo! Pero ninguna CPU conectada o libre aún! :( No hago nada hasta que no levanten una!");
			}

			pthread_cond_wait(&cond_ready_consumer, &mutex_ready_queue);
		}

		pthread_mutex_unlock(&mutex_ready_queue);
			log_info(logKernel, "Queue Manager Thread Says: Un nuevo proceso listo! Voy a pasarlo a ejecución!");
			agregarProcesoColaExec();

		pthread_cond_signal(&cond_ready_producer);

	}
}

void manejo_cola_exit(void){

	int myPid = 0; // DEJO 0 para probar
	log_info(logKernel, "************** Exit Manager Thread Started (PID: %d) ***************",myPid);

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
		mostrarColas();
		//-------------------------------------------------------------------------
	}
}


bool NoBodyHereBySemaphore(t_list* aList){

	bool _not_blocked_by_semaphore(void* element) {
		//((t_process*)element)->blockedBySemaphore == NO_SEMAPHORE
			if (string_equals_ignore_case(((t_process*)element)->blockedBySemaphore, NO_SEMAPHORE)) {
				return true;
			}
			return false;
		}

	return list_all_satisfy(aList, (void*)_not_blocked_by_semaphore);
}







