/*
 * Kernel.h
 *
 *  Created on: 07/05/2014
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

#ifdef COMPILAR_DESDE_CONSOLA

/*Commons includes*/
#include <Commons/commons/string.h>
#include <Commons/commons/log.h>
#include <Commons/commons/config.h>
#include <Commons/commons/collections/queue.h>
#include <Commons/commons/sockets.h>
#include <Commons/commons/process.h>

/*Parser includes*/
#include <Parser/metadata_program.h>

#else

/*Commons includes*/
#include "commons/string.h"
#include "commons/log.h"
#include "commons/config.h"
#include "commons/collections/queue.h"
#include "commons/sockets.h"
#include "commons/process.h"

/*Parser includes*/
#include "metadata_program.h"

#endif


//Internal variables
#define KernelLogPath "kernel.log"
#define QueueLogPath "queues.log"
//t_log *kernelLog;
int32_t PUERTO;
char* NO_SEMAPHORE;
char* NO_IO;

int32_t socketUMV;
t_list* ioList;
//t_dictionary* semaphoreDictionary;

/*FALGS PARA CONDITIONAL THREADS*/
unsigned event_flags;
#define FLAG_EVENT_EXIT  1
#define FLAG_EVENT_READY 2

//Estructura de PCB.
typedef struct {
		/*Tamanio del contexto actual. Cantidad de variables existentes
		* En el contexto de ejecución actual. Sirve para recrear el Diccionario de variables
		* al reanudar la ejecución de un programa.
		* Cuando un Programa se vuelve a planificar y debe reanudar su ejecución en una CPU, necesita
		* conocer la CANTIDAD DE VARIABLES LOCALES que debe leer del segmento de Stack.
		*/
		int32_t			contextoActual_size;
		int32_t			pId;//Identificador único del Programa en el sistema
		int32_t			programCounter;//Número de la próxima instrucción a ejecutar
		t_size			indiceEtiquetas_size;//Cantidad de bytes que ocupa el Índice de etiquetas
		int32_t			indiceEtiquetas;//Dirección del primer byte en la UMV del Índice de Etiquetas
		int32_t		 	indiceCodigo;//Dirección del primer byte en la UMV del Índice de Código
		int32_t			segmentoCodigo;//Dirección del primer byte en la UMV del segmento de código
		int32_t			cursorStack;//Dirección del primer byte en la UMV del Contexto de Ejecución Actual
		int32_t			segmentoStack; //Direccion del primer byte del segmento stack

	} t_pcb;

typedef struct t_process_s{
	t_pcb* process_pcb;
	char* scriptCode;
	int32_t process_fd;
	int32_t process_weight;
	bool in_umv;
	char blockedBySemaphore[100];
	char blockedByIO[100];
	int32_t io_tiempo;
}t_process;

typedef struct pcp {
	pthread_t tid;
	//int32_t fdPipe[2];  fdPipe[0] de lectura/ fdPipe[1] de escritura (Prueba)
} t_pcpThread;

typedef struct plp {
	pthread_t tid;
	//int32_t fdPipe[2];  fdPipe[0] de lectura/ fdPipe[1] de escritura (Prueba)
} t_plpThread;

typedef struct io{
	int32_t mutexes_consumer;
	pthread_t tid;
	char* nombre;
	int32_t retardo;
} t_iothread;

typedef struct ready_queue_manager{
	pthread_t tid;
} t_readyQueueManagerThread;

typedef struct exit_queue_manager{
	pthread_t tid;
} t_exitQueueManagerThread;

t_pcpThread pcpThread;
t_pcpThread plpThread;
t_readyQueueManagerThread readyQueueManagerThread;
t_exitQueueManagerThread exitQueueManagerThread;
t_iothread ioThread;

//Estructura para almacenar los valores del archivo de configuracion
typedef struct s_config_kernel{
	int32_t PUERTO_PROG;
	int32_t PUERTO_CPU;
	char* IP_UMV;
	int32_t PUERTO_UMV;
	int32_t QUANTUM;
	int32_t RETARDO_QUANTUM;
	int32_t MULTIPROG;
	char* ID_VARIABLES;
	char* VALOR_VARIABLES;
	char* SEMAFOROS;
	char* VALOR_SEMAFOROS;
	char* IO_RETARDO;
	char* IO_ID;
	int32_t SELF_P;
	int32_t STACK_SIZE;
} t_config_kernel;

typedef struct s_client_cpu{
	int32_t processFd;
	int32_t processPID;
	int32_t cpuPID;
	int32_t cpuFD;
	bool isBusy;
}t_client_cpu;

typedef struct s_var_compartida{
	char Id[30];
	int32_t Valor;
}t_var_compartida;

typedef struct s_semaforoAnsisop{
	char Id[100];
	int32_t Valor;
} t_semaforoAnsisop;

t_config_kernel config_kernel;

pthread_cond_t cond_exit_consumer, cond_exit_producer,cond_ready_consumer, cond_ready_producer, condpBlockedProcess;

t_list *cpu_client_list;
t_list* var_compartida_list;
t_list* semaforosAnsisop_list;

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

void *get_in_addr(struct sockaddr *sa);
void StartKernel(char* configPath);
void LoadConfig(char* configPath);

//hilo plp
void* plp(t_plpThread *plpThread);

//hilo plp
void* pcp(t_plpThread *plpThread);

//hilo manage_queue
void exit_queue_manager(void);
void ready_queue_manager(void);

//Metodos process_queue_manager
t_process* GetProcessStructureByAnSISOPCode(char* stringCode, int32_t PID, int32_t fd);
bool ProcessLessWeightComparator(void* elementA, void* elementB);
int32_t GetProcessWeightByProperties(int32_t cantidadEtiquetas, int32_t cantidadFunciones, int32_t cantidadLineasCodigo);
int ObtenerCantidadDeProcesosEnSistema();
bool FindProcessByFd(void *element, int32_t processFd);
char* FillNumberWithZero(char* number, int32_t fullSize);

void PrintQueues();
void PrintQueue(t_queue* aQueue, pthread_mutex_t queueMutex, t_log* logger);

void addNewProcess(t_process* aProcess);
void addReadyProcess(t_process* aProcess);
void addExecProcess();
void addExitProcess(t_process* aProcess);
void addBlockedProcess(int32_t processFd, char* semaphoreKey, char* ioKey, int32_t io_tiempo);

int32_t GetProcessPidByFd(int32_t fd);
void removeProcess(int32_t processFd, bool someoneKilledHim);
void enviarAEjecutar(int32_t socketCPU, int32_t  quantum, t_process* aProcess);
//void DestruirSegmentosEnUMV(int32_t victimPid);
void CheckNewProcesses();

//Métodos para hablar con la UMV
void ComunicarMuertePrograma(int32_t processFd, bool wasInUmv);
void EliminarSegmentos(int32_t pID);
void HandshakeUMV();
bool SolicitarSegmentosUMV(t_process* aProcess);
bool EscribirSegmentosUMV(t_process* aProcess);
void PedirSegmento(int32_t pID, int32_t tamanio);

void* io(t_iothread* ioThread);

//void PrintReadyQueue();
bool NoBodyHereBySemaphore(t_list* aList);
bool SomebodyHereByIO(t_list* aList, char* ioKey);
bool StillInside(int32_t processFd);

t_iothread* GetIOThreadByName(char* ioName);
void KillProcess(t_process* aProcess);
#endif /* KERNEL_H_ */

bool _match_pid(void* element);
bool _cpuIsFree(void* element);
t_client_cpu* GetCPUByCPUFd(int32_t cpuFd);
t_client_cpu* GetCPUReady();
