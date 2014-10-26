/*
 * funciones_CPU.h
 *
 *  Created on: 01/10/2014
 *      Author: utnso
 */

#ifndef FUNCIONES_CPU_H_
#define FUNCIONES_CPU_H_

#define LOAD "LOAD"
#define GETM "GETM"
#define SETM "SETM"
#define MOVR "MOVR"
#define ADDR "ADDR"
#define SUBR "SUBR"
#define MULR "MULR"
#define MODR "MODR"
#define DIVR "DIVR"
#define INCR "INCR"
#define DECR "DECR"
#define COMP "COMP"
#define CGEQ "CGEQ"
#define CLEQ "CLEQ"
#define GOTO "GOTO"
#define JMPZ "JMPZ"
#define JPNZ "JPNZ"
#define INTE "INTE"
#define FLCL "FLCL"



#include "cpu.h"
#include "instrucciones_CPU.h"


void iniciarPrograma();

void liberar_estructuras_CPU();

/**
		* @NAME: buscador_de_instrucciones
		* @DESC: Verifica cual es la instruccion
		* recibida para ejecutar
		*/

void buscador_de_instruccion(char* instruccion);

char* recibir_Instruccion();

void cargar_diccionario();

void manejar_senial(int n);

void enviar_parametro(char* parametro);

void enviar_posicion_recibir_registro(char* parametro);

void enviar_parametro_recibir_registro(char* parametro);

void enviar_parametro_recibir_numero(char* parametro);

int32_t verificador_de_registro(char* valor);

void funcion_verificador_segundo_registro_GETM(int32_t *var,int32_t aux2);

#endif /* FUNCIONES_CPU_H_ */
