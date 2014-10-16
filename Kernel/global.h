/*
 * global.h
 *
 *  Created on: 12/09/2014
 *      Author: utnso
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#define IP "127.0.0.1"
#define PUERTO "6667"
#define PACKAGESIZE 1024
#define BACKLOG 5
#define PUERTO_CONEXION "6668"

pthread_t pthread_CPU,pthread_Proceso_Consola;



int32_t aux_1 ;

typedef struct TCB{
	int32_t PID;
	int32_t TID;
	int32_t KM;
	int32_t M;

} TCB;

TCB p1;

void *puntero_estructura_a_mandar;



int  p_HILO,p_HILO_CONSOLA;
int inicializar_Kernel_comunicacion_CPU();
int inicializar_Kernel_comunicacion_PROCESO_CONSOLA();

#endif /* GLOBAL_H_ */
