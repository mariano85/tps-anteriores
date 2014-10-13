/*
 * funciones_manejo_cola.c
 *
 *  Created on: 08/10/2014
 *      Author: utnso
 */

#include "kernel.h"
#include "funciones_manejo_cola.h"



void agregarProcesoColaNew(TCB* aProcess) {


		pthread_mutex_lock (&mutex_new_queue);
		queue_push(NEW, aProcess);
		pthread_mutex_unlock (&mutex_new_queue);
		puts("Un nuevo proceso se insertó en la cola NEW!");
		log_info(kernel_log, "Un nuevo proceso se insertó en la cola NEW!");








}

void agregarProcesoColaReady(TCB* aProcess) {
	TCB* process_aux;

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

	void mostrarColas(){

		log_debug(queue_log, "NEW QUEUE -");
		mostrarCola(NEW, mutex_new_queue, queue_log);
		log_info(queue_log, " ");
		log_info(queue_log, "READY QUEUE -");
		mostrarCola(READY, mutex_ready_queue, queue_log);
		log_info(queue_log, " ");
		log_info(queue_log, "EXEC QUEUE -");
		mostrarCola(EXEC, mutex_exec_queue, queue_log);
		log_info(queue_log, " ");
		log_info(queue_log, "BLOCK QUEUE -");
		mostrarCola(BLOCK, mutex_block_queue, queue_log);
		log_info(queue_log, " ");
		log_info(queue_log, "EXIT QUEUE -");
		mostrarCola(EXIT, mutex_exit_queue, queue_log);
		log_info(queue_log, " ");



	}

	bool ordernRoundRobin(void* p1, void* p2){
		TCB* processA_aux = (TCB*)p1;
		TCB* processB_aux = (TCB*)p2;

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
				TCB* aProcess = (TCB*)list_get(list_aux, counter);
				log_info(logger, "-----------------%d-----------%d------------%d---------------", aProcess->pid, aProcess->tamanio_indice_codigo, aProcess->program_counter);
			}
			log_info(logger, "*************************************************************");
		}
		else{
			log_debug(logger, "No hay proesos :(!\n");
		}
	}

	void agregarProcesoColaExec(){

			t_client_cpu* aCpu;
			TCB* aProcess;
			bool disponible = false;

			pthread_mutex_lock(&mutexCPUDISP);
			log_debug(kernel_log, "BUSCO CPU LIBRE PARA EJECUTAR PROGRAMA!");

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
			sem_post(&mutexCPUDISP);


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
				log_info(kernel_log, string_from_format("Un nuevo programa entra en ejecución (PID: %d) en Procesador PID: %d", aProcess->pid, aCpu->cpuPID));

				enviarAEjecutar(aCpu->cpuFD, 10, aProcess); //DEJO 10 para poner un quantum

				mostrarColas();
			}
			else{
				log_info(kernel_log, "Se intentó poner en ejecución un nuevo programa, pero no había ninguno en READY. :/");
				//log_info(archivo_logs, string_from_format("CPU (PID: %d) aguarda instrucciones!", aCpu->cpuPID));
			}
		}
		else{
			//log_info(archivo_logs, string_from_format("Se intentó poner en ejecución un nuevo programa, pero no había ningun CPU libre o activo! :/"));
		}
	}


	void agregarProcesoColaExit(TCB* aProcess){

		pthread_mutex_lock(&mutex_exit_queue);	/* protect buffer */
		while (EXIT->elements->elements_count != 0){/* If there is something in the buffer then wait */
		  pthread_cond_wait(&cond_exit_producer, &mutex_exit_queue);
		}
		  //Makes things happen here! Inserts a new finished process into exit queue!
			log_debug(kernel_log, "Un nuevo proceso (PID: %i) ha finalizado. Se inserta en la cola de EXIT :", aProcess->pid);
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
				log_info(kernel_log, "La cola de procesos nuevos Ready se encuentra llena..");
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
		if (((TCB*)element)->pid== processPID) {
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
	TCB* aProcess;
	bool Hit = false; //Verifica si no se mato el proceso mientras lo buscaba

	if((aProcess = (TCB*)list_find(READY->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(READY->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (TCB*)list_find(NEW->elements, (void*)_match_process_pid)) != NULL){
		wasNew = true;
		Hit = true;
		list_remove_by_condition(NEW->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (TCB*)list_find(BLOCK->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(BLOCK->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (TCB*)list_find(EXEC->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(EXEC->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (TCB*)list_find(EXIT->elements, (void*)_match_process_pid)) != NULL){
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


		//matarProceso(aProcess);
	}
}



void manejo_cola_ready(void){

	int myPid = 1; // probando
	log_info(kernel_log, "**************Ready Queue Manager Thread Started (PID: %d) ***************",myPid);
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
			log_info(kernel_log, "Queue Manager Thread Says: Un nuevo proceso listo! Pero ninguna CPU conectada o libre aún! :( No hago nada hasta que no levanten una!");
			}

			pthread_cond_wait(&cond_ready_consumer, &mutex_ready_queue);
		}

		pthread_mutex_unlock(&mutex_ready_queue);
			log_info(kernel_log, "Queue Manager Thread Says: Un nuevo proceso listo! Voy a pasarlo a ejecución!");
			agregarProcesoColaExec();

		pthread_cond_signal(&cond_ready_producer);

	}
}

void manejo_cola_exit(void){

	int myPid = 0; // DEJO 0 para probar
	log_info(kernel_log, "************** Exit Manager Thread Started (PID: %d) ***************",myPid);

	for(;;){

		pthread_mutex_lock(&mutex_exit_queue);	/* protect buffer */
		while (EXIT->elements->elements_count == 0)/* If there is nothing in the buffer then wait */
		{
			pthread_cond_wait(&cond_exit_consumer, &mutex_exit_queue);
		}

		/*Make things happen here!*/
		log_debug(kernel_log, "Exit Queue Manager Thread Says: Hey!I Got you dude!");
		int32_t counter = 0;
		for(counter = 0; counter < EXIT->elements->elements_count; counter ++ ){
			TCB* aProcess = (TCB*)queue_peek(EXIT);

			removeProcess(aProcess->pid, false);
		}
		pthread_cond_signal(&cond_exit_producer);	/* wake up producer */
		pthread_mutex_unlock(&mutex_exit_queue);	/* release the buffer */
		mostrarColas();
		//-------------------------------------------------------------------------
	}
}




