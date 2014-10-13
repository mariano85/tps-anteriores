
#include <semaphore.h>


#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

#include "global.h"

t_log* queueLog;

void* sacarCola(t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar);




void mostrarCola(t_queue* aQueue, pthread_mutex_t queueMutex, t_log* logger);
void mostrarColas();
void agregarProcesoColaNew(TCB* aProcess);
void agregarProcesoColaReady(TCB* aProcess);
void agregarProcesoColaExec();
void agregarProcesoColaExit(TCB* aProcess);
void manejo_cola_ready(void);
void chequearProcesos();
int32_t cantDeProcesosEnSistema();
void manejo_cola_ready(void);

pthread_cond_t cond_exit_consumer, cond_exit_producer,cond_ready_consumer, cond_ready_producer, condpBlockedProcess;


