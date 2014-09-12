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
#include <commons/temporal.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/bitarray.h>
#include <commons/temporal.h>
#include <commons/log.h>
#include <commons/config.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

t_log *logs;
t_config* config;
t_dictionary* diccionarioDeVariables;

//Registro CPU despues veremos como los agrupamos

int  *A,*B,*C,*D,*E;
char *EFLAG;

int socketKernel;
int socketMSP;
int PUERTO_KERNEL,PUERTO_MSP;
pthread_t pthread_Consola;




int conectar_kernel();
int conectar_MSP();
int  p_HILO;

void inicializar_Configuracion();

void LOAD(int *Registro,int Numero);

void GETM(int *Registro1,int *Registro2);

void ADDR(int *Registro1,int *Registro2);

void SUBR(int *Registro1,int *Registro2);

void MULR(int *Registro1,int *Registro2);

void MODR(int *Registro1,int *Registro2);

void DIVR(int *Registro1,int *Registro2);

void INCR(int *Registro1);

void DECR(int *Registro1);

void COMP(int *Registro1,int *Registro2);

void CGEQ(int *Registro1,int *Registro2);

void CLEQ(int *Registro1,int *Registro2);

void *manejo_consola(); //Voy a crear una mini consola para probar las cosas


#endif /* VARIABLESGLOBALES_FUNCIONES_H_ */
