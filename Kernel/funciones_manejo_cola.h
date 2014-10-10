
#include <semaphore.h>

#include "manejo_TCB.h"
#include "kernel.h"
t_log* queueLog;

void* sacarCola(t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar);
void ponerCola(registroPCB *unPCB, t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar);

void mostrarColas();
void mostrarCola(t_queue* aQueue, sem_t queueMutex, t_log* logger);
void agregarProcesoColaExec();
void agregarColaNew(TCB* aProcess);
void agregarProceso(TCB* aProcess);

