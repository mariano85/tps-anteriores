/*
 * kernel.h
 *
 *  Created on: 08/10/2014
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include "funciones_manejo_cola.h"
#include "global.h"


#define KERNEL_CONF "config.conf"
#define MSG_SIZE 256
#define QUANTUM "QUANTUM"

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


pthread_mutex_t mutex_new_queue;
pthread_mutex_t mutex_ready_queue;
pthread_mutex_t mutex_block_queue;
pthread_mutex_t mutex_exec_queue;
pthread_mutex_t mutex_exit_queue;

pthread_mutex_t mutexCPUDISP;

t_log *kernel_log;
t_log *queue_log;
t_config *config;

t_list *cpu_disponibles_list;

bool archivo_configuracion_valido();
void conectarse_Planificador();



t_dictionary *semaforos;

typedef struct t_semaforos {
	int valor;
	t_queue *cola;
	sem_t hayAlgo;
	sem_t mutex;
} t_semaforos;



typedef struct s_client_cpu{
	int32_t processFd;
	int32_t processPID;
	int32_t cpuPID;
	int32_t cpuFD;
	bool ocupado;
}t_client_cpu;

typedef char t_contenido[MSG_SIZE];

// PRUEBA

TCB *tcb1,*tcb2,*tcb3,*tcb4;



#endif /* KERNEL_H_ */
