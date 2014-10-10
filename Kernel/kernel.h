/*
 * kernel.h
 *
 *  Created on: 08/10/2014
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include<pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <semaphore.h>

#define KERNEL_CONF "config.conf"

t_queue *NEW;
t_queue *READY;
t_queue * EXEC;
t_queue *EXIT;
t_queue *BLOCK;
sem_t mutexNEW;
sem_t mutexREADY;
sem_t mutexEXEC;
sem_t mutexEXIT;
sem_t mutexBLOCK;

t_log *archivo_logs;
t_log *queue_log;
t_config *config;

bool archivo_configuracion_valido();
void conectarse_Planificador();


t_dictionary *semaforos;

typedef struct t_semaforos {
	int valor;
	t_queue *cola;
	sem_t hayAlgo;
	sem_t mutex;
} t_semaforos;

typedef struct TCB{
	 int pid;

		int tid;

		int indicador_modo_kernel;
		int base_segmento_codigo;
		int tamanio_indice_codigo ;
		int indice_codigo;
		int program_counter;
		int puntero_instruccion;
		int base_stack;
		int cursor_stack;
		int reg_programacion;
} TCB;






#endif /* KERNEL_H_ */
