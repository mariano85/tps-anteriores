/*
 * kernel.h
 *
 *  Created on: 11/10/2014
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_



/*System includes*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>

#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/process.h>

#include "commons/sockets.h"

//Internal variables
#define KERNEL_LOG_PATH "kernel.log"
#define QUEUE_LOG_PATH "queues.lHog"
#define KERNEL_CONFIG_PATH "kernel.conf"



#define MODO_KERNEL 1
#define MODO_USUARIO 0

t_log* queueLog;
t_log* logKernel;

int aux;

t_config* kernelConfig;
int socket_cpu;

typedef struct{

	char A;
	char B;
	char C;
	char D;
	char E;

}t_registros_de_programacion;


typedef struct{
    int pid;
	int tid;
	int indicador_modo_kernel;
	int base_segmento_codigo;
	int tamanio_segmento_codigo ;
	int indice_codigo;
	int program_counter;
	int base_stack;
	int cursor_stack;

	t_registros_de_programacion registros_de_programacion;


}t_tcb;


typedef struct{

	char A;
	char B;
	char C;
	char D;
	char E;

}t_registros_programacion;




typedef struct s_thread {
	t_tcb* tcb;
	int32_t process_fd;
	char blockedBySemaphore[100];
	bool existe_msp;
} t_process;

typedef struct s_config_kernel{
	int32_t PUERTO;
	char* IP_MSP;
	int32_t PUERTO_MSP;
	char* IP_CPU;
	int32_t PUERTO_CPU;
	int32_t QUANTUM;
	char* SYSCALLS;
} t_config_kernel;



t_config_kernel config_kernel;

int32_t socketMSP;
t_list *cpu_client_list;

t_queue *NEW;
t_queue *READY;
t_queue *BLOCK;
t_queue *EXEC;
t_queue *EXIT;
t_queue *SYSCALLS;

pthread_mutex_t mutex_new_queue;
pthread_mutex_t mutex_ready_queue;
pthread_mutex_t mutex_block_queue;
pthread_mutex_t mutex_exec_queue;
pthread_mutex_t mutex_exit_queue;

pthread_mutex_t mutex_cpu_list;
pthread_mutex_t mutexCPUDISP;

/*
 * hilo loader
 */
typedef struct loader {
	pthread_t tid;
	//int32_t fdPipe[2];  fdPipe[0] de lectura/ fdPipe[1] de escritura (Prueba)
} t_loaderThread;

typedef struct s_client_cpu{
	int32_t processFd;
	int32_t processPID;
	int32_t cpuPID;
	int32_t cpuFD;
	bool ocupado;
}t_client_cpu;

t_list *cpu_disponibles_list;

typedef struct s_semaforos{
	char Id[100];
	int32_t Valor;
} t_semaforos;

t_list* semaforos_list;

typedef char t_nombre_variable;
typedef t_nombre_variable* t_nombre_semaforo;


t_loaderThread loaderThread;
t_loaderThread planificadorThread;
t_loaderThread conectarsePlanificadorThread;
t_loaderThread manejoColaReadyThread;
t_loaderThread manejoColaExitThread;


// funciones del kernel
void finishKernel();
void initKernel();
void handshakeMSP();
void loadConfig();
void comunicarMuertePrograma(int32_t processFd, bool wasInUmv);
void eliminarSegmentos(int32_t pID) ;
void killProcess(t_process* aProcess);
void conectarse_Planificador();
t_client_cpu* GetCPUByCPUFd(int32_t cpuFd);
int32_t getProcessPidByFd(int32_t fd);
t_process* getProcessStructureByBESOCode(char* stringCode, int32_t PID, int32_t fd);

// el loader
void* loader(t_loaderThread *loaderThread);
void *get_in_addr(struct sockaddr *sa);

void* planificador(t_loaderThread *loaderThread);










/* FUNCIONES MANEJO COLA
 *
 */
t_log* queueLog;


void mostrarColas();
void mostrarCola(t_queue* aQueue, pthread_mutex_t queueMutex, t_log* logger);
void agregarProcesoColaNew(t_process* aProcess);
void agregarProcesoColaReady(t_process* aProcess);
void agregarProcesoColaExec();
void agregarProcesoColaExecEnPrimerLugar(t_process* aProcess);
void agregarProcesoColaExit(t_process* aProcess);
void agregarProcesoColaBlock(int32_t processFd, char* semaphoreKey);
void manejo_cola_ready(void);
void manejo_cola_exit(void);
void chequearProcesos();
int32_t cantDeProcesosEnSistema();
bool NoBodyHereBySemaphore(t_list* aList);
void removeProcess(int32_t processPID, bool someoneKilledHim);
void enviarAEjecutar(int32_t socketCPU, int32_t  quantum, t_process* aProcess);
bool stillInside(int32_t processFd);

pthread_cond_t cond_exit_consumer, cond_exit_producer,cond_ready_consumer, cond_ready_producer, condpBlockedProcess;


char* NO_SEMAPHORE;






/* SERVICIOS KERNEL
 *
 */
int32_t socketCPU;
void interrupcion(t_process* aProcess, int32_t dir_memoria);
void entrada_estandar(int32_t pid, char* tipo);
void salida_estandar(int32_t pid, char* tipo);
void crear_hilo(t_process* aProcess);
void join (int32_t tid1,int32_t tid2);
void bloquear(t_process* aProceess, int32_t id);
void despertar(int32_t id);



#endif /* KERNEL_H_ */

