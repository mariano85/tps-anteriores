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
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
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

sem_t mutexCPUDISP;
t_log *archivo_logs;
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

/*
 * headers del manejo del tcb
 */

#define MODO_KERNEL 1
#define MODO_USUARIO 0

typedef struct s_tcb {
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


} t_tcb;

typedef struct s_client_cpu{
	int32_t processFd;
	int32_t processPID;
	int32_t cpuPID;
	int32_t cpuFD;
	bool ocupado;
}t_client_cpu;

/*
 * headers del manejo de cola
 */

t_log* queueLog;

void* sacarCola(t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar);
void ponerCola(t_tcb *unPCB, t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar);

void mostrarColas();
void mostrarCola(t_queue* aQueue, sem_t queueMutex, t_log* logger);


void agregarProceso(t_tcb* aProcess);
void agregarProcesoColaReady(t_tcb* aProcess);
void agregarProcesoColaExec();
void manejo_cola_ready(void);
void chequearProcesos();
int32_t cantDeProcesosEnSistema();

pthread_cond_t cond_exit_consumer, cond_exit_producer,cond_ready_consumer, cond_ready_producer, condpBlockedProcess;




#endif /* KERNEL_H_ */
