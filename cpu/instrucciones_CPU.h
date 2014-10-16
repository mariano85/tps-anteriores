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

void instruccion_FLCL();

void instruccion_MALC();

char* solicitar_rutina_kernel(int32_t Direccion);



#endif /* INSTRUCCIONES_CPU_H_ */
