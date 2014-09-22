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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PUERTO "6667"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024// Define cual va a ser el size maximo del paquete a enviar


typedef struct TCB{
	int32_t PID; //Identificador del proceso
	int32_t TID; // Identificador del hilo del proceso
	/*int32_t KM;  //Indicador de Modo
	int32_t *M;   // Base del segmento de codigo
	int32_t T;   //  Tamaño del segmento de codigo
	int32_t *P;   // Puntero de instruccion
	int32_t *X;	// Base del stack
	int32_t *S;	// Cursor del stack
	int32_t A;	// Registros de programacion
	int32_t B;
	int32_t C;
	int32_t D;
	int32_t E;*/
} TCB;


char *puntero_estructura_a_recibir;


//t_log *logs;
//t_config* config;
//t_dictionary* diccionarioDeVariables;

//Registro CPU despues veremos como los agrupamos

int  *A,*B,*C,*D,*E;
int *PC; //Program Counter
char *EFLAG;

int socketKernel;
int socketMSP;
int PUERTO_KERNEL,PUERTO_MSP;

pthread_t pthread_Consola;

int conectar_kernel();
int conectar_MSP();
int  p_HILO;

int aux = 32;

char texto_entrada;

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

void GOTO(int *Registro);

void manejo_consola(); //Voy a crear una mini consola para probar las cosas

void consola();

int inicializar_CPU_conexion_kernel();

#endif /* VARIABLESGLOBALES_FUNCIONES_H_ */
