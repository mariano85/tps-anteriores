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
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <semaphore.h>

#define IP "127.0.0.1"
#define PUERTO "6667"
#define PACKAGESIZE 1024
#define BACKLOG 5
#define PUERTO_CONEXION "6668"

pthread_t pthread_CPU,pthread_Proceso_Consola;



int32_t aux_1 ;

typedef struct TCB{
	 	 int pid;

		int tid;

		int indicador_modo_kernel;
		int base_segmento_codigo;
		int tamanio_indice_codigo ;
		int indice_codigo;
		int program_counter;
		int puntero_instruccion;
		int base_stack;
		int cursor_stack;
		int reg_programacion;
} TCB;

TCB p1;

void *puntero_estructura_a_mandar;

TCB* tcb_puntero_estructura;



int  p_HILO,p_HILO_CONSOLA;
int inicializar_Kernel_comunicacion_CPU();
int inicializar_Kernel_comunicacion_PROCESO_CONSOLA();

#endif /* GLOBAL_H_ */
