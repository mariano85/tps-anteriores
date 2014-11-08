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


	pthread_mutex_lock(&mutex_new_queue);
	if((aProcess = (t_process*)list_find(NEW->elements, (void*)_match_pid_tid)) != NULL){
		pthread_mutex_unlock(&mutex_new_queue);
		return aProcess->tcb->pid;
	}
	else{
		pthread_mutex_unlock(&mutex_new_queue);
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
				log_info(logger, "-----------------%d-----------%d------------%d---------------", aProcess->pid, aProcess->tamanio_segmento_codigo, aProcess->program_counter);
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


void agregarProcesoKernel(t_process* aProcess) {
	pthread_mutex_lock (&mutex_block_queue);
	queue_push(BLOCK, aProcess);
	pthread_mutex_unlock (&mutex_block_queue);
	log_info(logKernel, "Se inserta el proceso de Kernel en la cola BLOCK!");
}


void agregarProcesoColaReady(t_process* aProcess) {

	t_process* process_aux;


		pthread_mutex_lock (&mutex_new_queue);
					/*Ordeno cola de NEW por algoritmo de SJN*/

					t_list* list_aux;
					list_aux =  NEW->elements;
					NEW->elements = list_aux;

					/*Extraigo el primero mas liviano*/
					process_aux = queue_pop(NEW);
		pthread_mutex_unlock (&mutex_new_queue);



	pthread_mutex_lock(&mutex_ready_queue);	/* Blocks the buffer */
			queue_push(READY, process_aux);
			mostrarColas();
	pthread_mutex_unlock(&mutex_ready_queue);	/* release the buffer */
}



void agregarProcesoColaExec(){

	t_client_cpu* aCpu;
	t_process* aProcess;
	bool disponible = false;
	log_info(logKernel, "estoy en agregarproceso cola exxec pero me trabe!");

	pthread_mutex_lock(&mutexCPUDISP);
	log_debug(logKernel, "BUSCO CPU LIBRE PARA EJECUTAR PROGRAMA!");



	if(list_any_satisfy(cpu_disponibles_list, (void*)cpuLibre)){
		aCpu = list_find(cpu_disponibles_list, (void*) cpuLibre);
		disponible = true;
	}
	pthread_mutex_unlock(&mutexCPUDISP);


if(disponible){
	pthread_mutex_lock (&mutex_ready_queue);
	log_info(logKernel, "entre al if!");

	log_info(logKernel, "agrego un proceso de prueba!");
		aProcess = queue_pop(READY);
		log_info(logKernel, "saque algo de la cola ready !");
		pthread_mutex_unlock (&mutex_ready_queue);

	if(aProcess != NULL){ // ACA REVISIO SI ES UNA LLAMADA AL SISTEMA, SI LO ES LO AGREGO AL PRINCIPIO DE LA COLA EXECUTE

		//int chequeoModoKernel = aProcess ->tcb->indicador_modo_kernel; //////LO DEJOCOMENTADO PARA PROBARR
		int chequeoModoKernel = 0; // ESTO HAY QUE SACARLO
		pthread_mutex_lock (&mutex_exec_queue);


							if(chequeoModoKernel == 0){
								queue_push(EXEC, aProcess);
								log_info(logKernel, "puse un proceso en exec!");
							}
							else{
								int posicion = 0;
								list_add_in_index(EXEC -> elements, posicion,aProcess);
							}


							pthread_mutex_unlock (&mutex_exec_queue);







		aCpu->ocupado = true;
		aCpu->processPID = aProcess->tcb->pid;



		//log_debug(kernelLog,("Un nuevo programa entra en ejecución (PID: %d) en Procesador PID: %d", aProcess->pid, aCpu->cpuPID));

										log_info(logKernel, "Estoy ejecutando algo !lo envie a una cpu!");
										mostrarColas();
										enviarAEjecutar(aCpu->cpuFD, config_kernel.QUANTUM, aProcess);

		mostrarColas();
	}
	else{
		log_info(logKernel, "Se intentó poner en ejecución un nuevo programa, pero no había ninguno en READY. :/");
	//	log_info(logKernel, string_from_format("CPU (PID: %d) aguarda instrucciones!", aCpu->cpuPID));
	}
}

else{
	log_info(logKernel, "Se intentó poner en ejecución un nuevo programa, pero no había ningun CPU libre o activo! :/");
}
}

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
		if(!someoneKilledHim){/*Si nadie lo mató, entonces entró en la cola de EXIT a*/
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

	int myPid = process_get_thread_id();
	log_info(logKernel, "**************Manejo cola ready (PID: %d) ***************",myPid);
	bool hayCpuLibre = false;
	t_client_cpu* cpu;
	t_process* aProcess = malloc(sizeof(t_process));
	log_info(logKernel, "**************Paso a ready los procesos de la cola new****",myPid);



	//PARA IR VERIFICANDO LA COLA NEW

	for(;;){
		pthread_mutex_lock(&mutex_new_queue);
		while (NEW->elements->elements_count == 0){

		}
		pthread_mutex_unlock(&mutex_new_queue);
		log_info(logKernel, "Queue Manager Thread Says: Un nuevo proceso en new! Voy a pasarlo a ready!");
		agregarProcesoColaReady(aProcess);

		pthread_cond_signal(&cond_new_producer);

	}



	for(;;){
		hayCpuLibre = false;

		pthread_mutex_lock(&mutex_ready_queue);



		while (READY->elements->elements_count == 0 ||

					(!list_any_satisfy(cpu_disponibles_list, (void*)cpuLibre)
						&& READY->elements->elements_count != 0))//
		{
			if(!list_any_satisfy(cpu_disponibles_list, (void*)cpuLibre)
					&& READY->elements->elements_count != 0){
			log_info(logKernel, "Manejo cola dice: Un nuevo proceso listo! Pero ninguna CPU conectada o libre aún! :( No hago nada hasta que no levanten una!");
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

	int myPid = process_get_thread_id(); 	log_info(logKernel, "************** Manejo cola exit (PID: %d) ***************",myPid);

	for(;;){

		pthread_mutex_lock(&mutex_exit_queue);	/* protect buffer */
		while (EXIT->elements->elements_count == 0)/* If there is nothing in the buffer then wait */
		{
			pthread_cond_wait(&cond_exit_consumer, &mutex_exit_queue);
		}

		/*Make things happen here!*/
		log_debug(logKernel, "Hay un proceso ! Vamos a sacarlo!");
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



bool cpuLibre(void* element){

						if(((t_client_cpu*)element)->ocupado == false){
							return true;
						}
						return false;
					}



