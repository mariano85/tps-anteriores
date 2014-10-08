
#include <semaphore.h>

#include "manejo_TCB.h"
#include "kernel.h"

void* sacarCola(t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar);
void ponerCola(registroPCB *unPCB, t_queue* cola_actual, sem_t *mutex, sem_t *hay_algo_para_sacar);

