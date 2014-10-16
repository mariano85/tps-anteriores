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

#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/process.h>
#include <commons/sockets.h>

//Internal variables
#define KERNEL_LOG_PATH "kernel.log"
#define QUEUE_LOG_PATH "queues.lHog"
#define KERNEL_CONFIG_PATH "kernel.conf"
#define CONFIG_KERNEL ""

#define MODO_KERNEL 1
#define MODO_USUARIO 0

typedef struct s_tcb{
    int32_t pid;
	int32_t tid;
	bool indicador_modo_kernel;
	int32_t base_segmento_codigo;
	uint32_t tamanio_indice_codigo ;
	int32_t indice_codigo;
	int32_t program_counter;
	int32_t puntero_instruccion;
	int32_t base_stack;
	int32_t cursor_stack;

	int reg_programacion;
} t_tcb;

typedef struct s_thread {
	t_tcb* tcb;
	int32_t process_fd;
	bool in_umv;
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

int32_t socketUMV;
t_list *cpu_client_list;

t_queue *new_queue;
t_queue *ready_queue;
t_queue *block_queue;
t_queue *exec_queue;
t_queue *exit_queue;

pthread_mutex_t mutex_new_queue;
pthread_mutex_t mutex_ready_queue;
pthread_mutex_t mutex_block_queue;
pthread_mutex_t mutex_exec_queue;
pthread_mutex_t mutex_exit_queue;

pthread_mutex_t mutex_cpu_list;


/*
 * hilo loader
 */
typedef struct loader {
	pthread_t tid;
	//int32_t fdPipe[2];  fdPipe[0] de lectura/ fdPipe[1] de escritura (Prueba)
} t_loaderThread;

t_loaderThread loaderThread;

// funciones del kernel
void finishKernel();
void initKernel();
void handshakeMSP();
void loadConfig();
void comunicarMuertePrograma(int32_t processFd, bool wasInUmv);
void eliminarSegmentos(int32_t pID) ;
void killProcess(t_process* aProcess);

// el loader
void* loader(t_loaderThread *loaderThread);

//Metodos process_queue_manager
bool stillInside(int32_t processFd);
int32_t getProcessPidByFd(int32_t fd);
t_process* getProcessStructureByBESOCode(char* stringCode, int32_t PID, int32_t fd);

void checkNewProcesses();

void printQueues();
void printQueue(t_queue* aQueue, pthread_mutex_t queueMutex, t_log* logger);

void addNewProcess(t_process* aProcess);
void addReadyProcess(t_process* aProcess);
void addExecProcess();
void addExitProcess(t_process* aProcess);
void addBlockedProcess(int32_t processFd, char* semaphoreKey, char* ioKey, int32_t io_tiempo);
void removeProcess(int32_t processPID, bool someoneKilledHim);

#endif /* KERNEL_H_ */

