/*
 * proyectoPractica.h
 *
 *  Created on: 29/08/2014
 *      Author: utnso
 */

#ifndef VARIABLESGLOBALES_FUNCIONES_H_
#define VARIABLESGLOBALES_FUNCIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/process.h>

#include <commons/collections/dictionary.h>

#include <semaphore.h>
#include <strings.h>

#include "funciones_CPU.h"
#include "instrucciones_CPU.h"
#include "commons/sockets.h"

#define MODO_KERNEL 1
#define MODO_USUARIO 0


#define registro_A "A"
#define registro_B "B"
#define registro_C "C"
#define registro_D "D"
#define registro_E "E"

#define PROGRAMA_CONF_PATH "configuracion.conf"
#define PUERTO_KERNEL "PUERTO_KERNEL"
#define PUERTO_MSP "PUERTO_MSP"
#define IP_MSP "IP_MSP"
#define IP_KERNEL "IP_KERNEL"
#define RETARDO "RETARDO"
#define PUERTO "6667"
#define PACKAGESIZE 1024// Define cual va a ser el size maximo del paquete a enviar

#define ZERO_DIV 1
#define FLAG_VACIO 0

#define TAMANIO_PAGINA 256

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
	int tamanio_segmento_stack;
	int puntero_instruccion;
	int base_stack;
	int cursor_stack;
	int cantidad_de_hijos_creados;
	int contador_de_hijos_activos;
	int fd;

	t_registros_de_programacion registros_de_programacion;


}registro_TCB;

registro_TCB *TCB;



t_dictionary *diccionarioDeVariables;
t_log *logs;
t_config* config;
int seguir;
int32_t puertoKernel;
int32_t MODO;

//Registro CPU despues veremos como los agrupamos

int32_t A,B,C,D,E,M,P,X,S;
int EFLAG;
int ejecutando;
int systemCall;

int socketMSP;
int socketKernel;
int	p_HILO;
int32_t program_counter;
char* ipKernel  ;
int32_t QUANTUM;
int cont;
int termino_proceso_XXXX;
int newFD;

sem_t mutex_A;
sem_t mutex_B;
sem_t mutex_C;
sem_t mutex_D;
sem_t mutex_E;

pthread_t pthread_Consola;

char texto_entrada;

uint32_t generarDireccionLogica(int numeroSegmento, int numeroPagina, int offset);

void obtenerUbicacionLogica(uint32_t direccion, int *numeroSegmento, int *numeroPagina, int *offset);

uint32_t aumentarProgramCounter(uint32_t programCounterAnterior, int bytesASumar);

void conexion_Kernel();

void enviar_parametros(int32_t program_counter,int32_t auxiliar);

void estado_registros();


#endif /* VARIABLESGLOBALES_FUNCIONES_H_ */
