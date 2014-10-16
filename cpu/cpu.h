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

#include <string.h>

#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <commons/config.h>

#include <commons/collections/dictionary.h>

#include <semaphore.h>
#include <strings.h>

#include "funciones_CPU.h"
#include "instrucciones_CPU.h"
#include "sockets.h"


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

typedef struct{

	int32_t A;
	int32_t B;
	int32_t C;
	int32_t D;
	int32_t E;

}t_registros_de_programacion;

typedef struct{
    int pid;

	int tid;

	int indicador_modo_kernel;
	int base_segmento_codigo;
	int tamanio_indice_codigo ;
	int indice_codigo;
	int puntero_instruccion;
	int base_stack;
	int cursor_stack;

	t_registros_de_programacion registros_de_programacion;


}registro_TCB;

registro_TCB *TCB;



t_dictionary *diccionarioDeVariables;
t_log *logs;
t_config* config;



//Registro CPU despues veremos como los agrupamos

int  A,B,C,D,E;
int EFLAG;
int ejecutando;
int systemCall;
int socketKernel;
int socketMSP;
int  p_HILO;
int program_counter;

sem_t mutex_A;
sem_t mutex_B;
sem_t mutex_C;
sem_t mutex_D;
sem_t mutex_E;

pthread_t pthread_Consola;

char texto_entrada;

#endif /* VARIABLESGLOBALES_FUNCIONES_H_ */
