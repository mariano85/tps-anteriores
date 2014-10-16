/*
 * funciones_manejo_cola.c
 *
 *  Created on: 08/10/2014
 *      Author: utnso
 */

#include "kernel.h"

/**
 * NOTA: los semaforos no tienen que ir por parametro, ya que son variables GLOBALES
 */

void* sacarCola(t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar){
	sem_wait(hay_algo_para_sacar);
	sem_wait(mutex);
	t_tcb* unPCB = queue_pop(cola_actual);
	sem_post(mutex);

	return unPCB;
}
void ponerCola(t_tcb *unPCB, t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar){

	sem_wait(mutex);
	queue_push(cola_actual, unPCB);
	sem_post(mutex);

	sem_post(hay_algo_para_sacar);
}

void agregarProceso(t_tcb* aProcess) {
		int32_t SystemProcessCounter = 0;
		SystemProcessCounter = 5; //cantDeProcesosEnSistema() PERO NO ANDA ¬¬

		if(10 > SystemProcessCounter){ //PONEMOS UN NUMERO PERO IRIA config_kernel.MULTIPROG
				agregarProcesoColaReady(aProcess);
			}
		else{

		sem_close(&mutexNEW);
		queue_push(NEW, aProcess);
		sem_post(&mutexNEW);
		puts("Un nuevo proceso se insertó en la cola de listos!");
		log_info(archivo_logs, "Se intentó encolar un nuevo proceso listo(Ready), pero el Sistema alcanzó el máximo de programas según grado de multiprogramación.");
		log_info(archivo_logs, "Un nuevo proceso se insertó en la cola de listos!");

		// mostrarColas(); TAMPOCO ANDA !

		}




}

void agregarProcesoColaReady(t_tcb* aProcess) {
	t_tcb* process_aux;

	if(aProcess != NULL){
		process_aux = aProcess;
	}
	else{
		sem_close (&mutexNEW);
			/*Ordeno cola de NEW por algoritmo de SJN*/

			t_list* list_aux;
			list_aux =  NEW->elements;
	//		list_sort(list_aux, (void*)ordernRoundRobin);   ///////// VER EL ORDEN QUE VAMOS A USAR

			NEW->elements = list_aux;

			/*Extraigo el primero mas liviano*/
			process_aux = queue_pop(NEW);
		sem_post (&mutexNEW);
	}

	void mostrarColas(){

		log_info(queueLog, "NEW QUEUE -");
		mostrarCola(NEW, mutexNEW, queueLog);
		log_info(queueLog, " ");
		log_info(queueLog, "READY QUEUE -");
		mostrarCola(READY, mutexREADY, queueLog);
		log_info(queueLog, " ");
		log_info(queueLog, "EXEC QUEUE -");
		mostrarCola(EXEC, mutexEXEC, queueLog);
		log_info(queueLog, " ");
		log_info(queueLog, "BLOCK QUEUE -");
		mostrarCola(BLOCK, mutexBLOCK, queueLog);
		log_info(queueLog, " ");
		log_info(queueLog, "EXIT QUEUE -");
		mostrarCola(EXIT, mutexEXIT, queueLog);
		log_info(queueLog, " ");



	}

	bool ordernRoundRobin(void* p1, void* p2){
		t_tcb* processA_aux = (t_tcb*)p1;
		t_tcb* processB_aux = (t_tcb*)p2;

		return processA_aux->pid < processB_aux->pid;
	}



	void mostrarCola(t_queue* aQueue, sem_t *queueMutex, t_log* logger){

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
			log_info(logger, "There is any process here yet!\n");
		}
	}

	void agregarProcesoColaExec(){

			t_client_cpu* aCpu;
			t_tcb* aProcess;
			bool disponible = false;

			sem_close(&mutexCPUDISP);
		//	log_debug(kernelLog, "BUSCO CPU LIBRE PARA EJECUTAR PROGRAMA!");
			//if(list_any_satisfy(cpu_disponibles_list, (void*)_cpuLibre)){
				//aCpu = list_find(cpu_disponibles_list, (void*) _cpuLibre);
				//disponible = true;
			//}
			sem_post(&mutexCPUDISP);


		if(disponible){
			sem_close(&mutexREADY);

				aProcess = queue_pop(READY);
			sem_post (&mutexREADY);

			if(aProcess != NULL){

				sem_close (&mutexEXEC);
					queue_push(EXEC, aProcess);
				sem_post (&mutexEXEC);


				aCpu->ocupado = true;
				aCpu->processPID = aProcess->pid;
	//			log_info(archivo_logs, string_from_format("Un nuevo programa entra en ejecución (PID: %d) en Procesador PID: %d", aProcess->pid, aCpu->cpuPID));

			//	enviarAEjecutar(aCpu->cpuFD, 10, aProcess); //DEJO 10 para poner un quantum

				mostrarColas();
			}
			else{
				log_info(archivo_logs, "Se intentó poner en ejecución un nuevo programa, pero no había ninguno en READY. :/");
				//log_info(archivo_logs, string_from_format("CPU (PID: %d) aguarda instrucciones!", aCpu->cpuPID));
			}
		}
		else{
			//log_info(archivo_logs, string_from_format("Se intentó poner en ejecución un nuevo programa, pero no había ningun CPU libre o activo! :/"));
		}
	}

	void chequearProcesos(){
		//if(config_kernel.MULTIPROG > ObtenerCantidadDeProcesosEnSistema()){
			if(queue_size(NEW) > 0){
				agregarProcesoColaReady(NULL);
			}
			else{
				log_info(archivo_logs, "La cola de procesos nuevos (New-Queue) se encuentra vacía...");
			}
		//}
	}

	int32_t cantDeProcesosEnSistema(){

			int32_t counter = 0;

			sem_close (&mutexBLOCK);
			sem_close (&mutexREADY);
			sem_close (&mutexEXEC);
				counter += READY->elements->elements_count;
				counter += EXEC->elements->elements_count;
				counter += BLOCK->elements->elements_count;
			sem_post  (&mutexEXEC);
			sem_post  (&mutexREADY);
			sem_post  (&mutexBLOCK);

			//log_debug(archivo_logs, ("Cantidad de procesos en el sistema: %d", counter));

			return counter;
	}

}














