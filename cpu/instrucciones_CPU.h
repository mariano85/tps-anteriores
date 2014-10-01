/*
 * instrucciones_CPU.h
 *
 *  Created on: 01/10/2014
 *      Author: utnso
 */

#ifndef INSTRUCCIONES_CPU_H_
#define INSTRUCCIONES_CPU_H_

#include "cpu.h"

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

#endif /* INSTRUCCIONES_CPU_H_ */
