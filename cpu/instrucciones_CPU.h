/*
 * instrucciones_CPU.h
 *
 *  Created on: 01/10/2014
 *      Author: utnso
 */

#ifndef INSTRUCCIONES_CPU_H_
#define INSTRUCCIONES_CPU_H_

#include "cpu.h"

#define MALC "MALC"
#define FREE "FREE"
#define INNN "INNN"
#define INNC "INNC"
#define OUTN "OUTN"
#define CREA "CREA"
#define JOIN "JOIN"
#define BLOK "BLOK"
#define WAKE "WAKE"

void instruccion_LOAD(int32_t Registro,int32_t Numero);

void instruccion_GETM(int32_t Registro1,int32_t Registro2);

void instruccion_SETM(int32_t numero,int32_t Registro1,int32_t Registro2);

void instruccion_MOVR(int32_t Registro1,int32_t Registro2);

void instruccion_ADDR(int32_t Registro1,int32_t Registro2);

void instruccion_SUBR(int32_t Registro1,int32_t Registro2);

void instruccion_MULR(int32_t Registro1,int32_t Registro2);

void instruccion_MODR(int32_t Registro1,int32_t Registro2);

void instruccion_DIVR(int32_t Registro1,int32_t Registro2);

void instruccion_INCR(int32_t Registro1);

void instruccion_DECR(int32_t Registro1);

void instruccion_COMP(int32_t Registro1,int32_t Registro2);

void instruccion_CGEQ(int32_t Registro1,int32_t Registro2);

void instruccion_CLEQ(int32_t Registro1,int32_t Registro2);

void instruccion_GOTO(int32_t Registro);

void instruccion_JPMZ(int32_t Direccion);

void instruccion_JPNZ(int32_t Direccion);

void instruccion_INTE(int32_t Direccion);

void instruccion_MALC(int32_t direccion);

void instruccion_FREE(int32_t direccion);

void instruccion_INNN(int32_t direccion);

void instruccion_INNC(int32_t direccion);

void instruccion_OUTN(int32_t direccion);

void instruccion_CREA(int32_t direccion);

void instruccion_JOIN(int32_t direccion);

void instruccion_BLOK(int32_t direccion);

void instruccion_XXXX();

void instruccion_WAKE(int32_t direccion);

char* solicitar_rutina_kernel(int32_t Direccion);

void devolver_TCB();

#endif /* INSTRUCCIONES_CPU_H_ */
