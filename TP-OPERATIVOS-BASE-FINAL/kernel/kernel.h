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

#include <panel/commons/string.h>
#include <panel/commons/log.h>
#include <panel/commons/config.h>
#include <panel/commons/collections/queue.h>
#include <panel/commons/process.h>

#include <panel/commons/sockets.h>
#include <panel/panel.h>

//Internal variables
#define KERNEL_CONFIG_PATH "kernel.conf"
#define KERNEL_LOG_PATH "kernel.log"
#define QUEUE_LOG_PATH "queues.log"
#define LOADER_LOG_PATH "loader.log"
#define PCP_LOG_PATH "planificador.log"

#define KERNEL_PID -1
#define KERNEL_TID 0

#define MODO_KERNEL true
#define MODO_USUARIO false

t_config* kernelConfig;
t_log* logKernel;
t_log* logLoader;
t_log* logPlanificador;
t_log* queueLog;

typedef struct {
	t_hilo* tcb;
	int32_t process_fd;
	uint32_t direccion_syscall;
} t_process;

typedef struct {
	int32_t PUERTO;
	char* IP_MSP;
	int32_t PUERTO_MSP;
	char* IP_CPU;
	int32_t PUERTO_CPU;
	int32_t QUANTUM;
	char* SYSCALLS;
	int32_t TAMANIO_STACK;
} t_config_kernel;

/*
 * hilo loader
 */
typedef struct {
	pthread_t tid;
	//int32_t fdPipe[2];  fdPipe[0] de lectura/ fdPipe[1] de escritura (Prueba)
} t_thread;

typedef struct {
	int32_t cpuPID;
	int32_t cpuFD;
	t_process *procesoExec;
} t_client_cpu;

// TODO: migrarlo a un mapa... a un mapa!!!
typedef struct {
	int32_t id;
	int32_t valor;
	t_queue *colaBloqueados;
} t_semaforos;

t_config_kernel config_kernel;

t_process* procesoKernel;
int32_t socketMSP;
t_list *cpu_client_list;

t_queue *COLA_READY;
t_queue *COLA_SYSCALLS;
t_queue *COLA_EXIT;
t_queue *COLA_JOIN;

pthread_mutex_t mutex_ready_queue;
pthread_mutex_t mutex_syscalls_queue;
pthread_mutex_t mutex_join_queue;
pthread_mutex_t mutex_exec_queue;
pthread_mutex_t mutex_exit_queue;
pthread_mutex_t mutex_cpu_list;
pthread_mutex_t mutex_tcb_km;

t_list* semaforos_list;

t_thread loaderThread;
t_thread planificadorThread;
t_thread manejoColaReadyThread;
t_thread manejoColaExitThread;

pthread_cond_t cond_exit_consumer, cond_exit_producer,cond_ready_consumer, cond_ready_producer, cond_new_producer, cond_new_consumer,condpBlockedProcess;

// funciones del kernel
void finishKernel();
void initKernel();
void crearProcesoKM();
char* getBytesFromFile(FILE* entrada, size_t *tam_archivo);
int32_t escribirMemoria(int32_t pid, uint32_t direccionSegmento, char* buffer, int32_t tamanio);
uint32_t solicitarSegmento(int32_t id_proceso, uint32_t tamanio);
void handshakeMSP();
void loadConfig();
void comunicarMuertePrograma(int32_t processFd, bool wasInUmv);
void eliminarSegmentos(int32_t pID) ;
void killProcess(t_process* aProcess);
int32_t getProcessPidByFd(int32_t fd);

// el loader
void* loader(t_thread *loaderThread);
void *get_in_addr(struct sockaddr *sa);

void* planificador(t_thread *planificadorThread);
void actualizarTCB(t_process* aProcess, char* mensaje);
void eliminarCpu(int32_t socketCpu);
void agregarCpu(int32_t socketCpu, char* mensaje);
void manejarFinDeQuantum(int32_t socketCpu, char* mensaje);
void manejarFinDeProceso(int32_t socketCpu, char* mensaje);
t_client_cpu* buscarCPUPorFD(int32_t socketCpu);
t_process* desocuparCPU(int32_t socketCpu);

//LAS QUE AGREGUE YO
//t_client_cpu* encontrarCPUporFd(int32_t cpuFd);
t_process* getProcesoDesdeCodigoBESO(int32_t indicadorModo, char* codigoBESO, int32_t tamanioCodigo, int32_t PID, int32_t TID, int32_t fd);
int32_t solicitarSegmentoStack();
int32_t solicitarSegmentoCodigo();
void pruebasKernel();
void conexionCPU();

/**
 * FUNCIONES MANEJO COLA
 */
void setearProcesoCola(t_process* aProcess, t_cola cola);
void agregarProcesoColaNew(t_process* aProcess);
void agregarProcesoColaReady(t_process* aProcess);
void pushColaReady(t_process* aProcess);
void manejo_cola_ready();
void enviarAEjecutar(int32_t socketCPU, int32_t  quantum, t_process* aProcess);
void agregarProcesoColaExec();
void context_switch_ida();
t_process* context_switch_vuelta();
void agregarProcesoColaSyscall(t_process* unProceso, uint32_t direccion);
void agregarProcesoColaExit(t_process* aProcess);
bool cpuLibre(void* element);
void removeProcess(int32_t processPID, bool someoneKilledHim);
void manejo_cola_exit();
t_client_cpu* buscarCpuPorSocket(int32_t cpuFd);
bool stillInside(int32_t processFd);
int32_t encontrarProcesoPorFD(int32_t fd);
int32_t encontrarProcesoPorPIDyTID(int32_t pid, int32_t tid);


/* SERVICIOS KERNEL
 *
 */
void interrupcion(int32_t socketCpu, char* mensaje);
void entrada_estandar(int32_t socketCpu, char* mensaje);
void salida_estandar(int32_t socketCpu, char* mensaje);
void crear_hilo(int32_t socketCpu, char* mensaje);
void join (int32_t tid1,int32_t tid2);
void bloquear(t_process* aProceess, int32_t id);
void despertar(int32_t id);
void manejo_cola_exit();


#endif /* KERNEL_H_ */

