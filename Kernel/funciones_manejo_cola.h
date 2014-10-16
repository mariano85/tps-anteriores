
#include <semaphore.h>

#include "manejo_TCB.h"
#include "kernel.h"
t_log* queueLog;

void* sacarCola(t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar);
void ponerCola(t_tcb *unPCB, t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar);

void mostrarColas();
void mostrarCola(t_queue* aQueue, sem_t queueMutex, t_log* logger);


void agregarProceso(TCB* aProcess);
void agregarProcesoColaReady(TCB* aProcess);
void agregarProcesoColaExec();
void manejo_cola_ready(void);
void chequearProcesos();
int32_t cantDeProcesosEnSistema();

pthread_cond_t cond_exit_consumer, cond_exit_producer,cond_ready_consumer, cond_ready_producer, condpBlockedProcess;


