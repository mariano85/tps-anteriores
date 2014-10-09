/*
 * funciones_manejo_cola.c
 *
 *  Created on: 08/10/2014
 *      Author: utnso
 */


#include "funciones_manejo_cola.h"

void* sacarCola(t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar){
	sem_wait(hay_algo_para_sacar);
	sem_wait(mutex);
	registroPCB* unPCB = queue_pop(cola_actual);
	sem_post(mutex);

	return unPCB;
}
void ponerCola(registroPCB *unPCB, t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar){

	sem_wait(mutex);
	queue_push(cola_actual, unPCB);
	sem_post(mutex);

	sem_post(hay_algo_para_sacar);
}
