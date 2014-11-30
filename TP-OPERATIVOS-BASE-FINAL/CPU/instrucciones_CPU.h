/*
 * instrucciones_CPU.h
 *
 *  Created on: 01/10/2014
 *      Author: utnso
 */

#ifndef INSTRUCCIONES_CPU_H_
#define INSTRUCCIONES_CPU_H_

#include "cpu.h"

#include <math.h>

#define MALC "MALC"
#define FREE "FREE"
#define INNN "INNN"
#define INNC "INNC"
#define OUTN "OUTN"
#define CREA "CREA"
#define JOIN "JOIN"
#define BLOK "BLOK"
#define WAKE "WAKE"
#define XXXX "XXXX"

void instruccion_LOAD(int32_t *Registro,int32_t Numero); //ANDA

void instruccion_GETM(int32_t *Registro1,int32_t *Registro2); //ANDA

void instruccion_SETM(int32_t numero,int32_t Registro1,int32_t Registro2);

void instruccion_MOVR(int32_t *Registro1,int32_t *Registro2);

void instruccion_ADDR(int32_t *Registro1,int32_t *Registro2); //ANDA

void instruccion_SUBR(int32_t *Registro1,int32_t *Registro2); //ANDA

void instruccion_MULR(int32_t *Registro1,int32_t *Registro2); //ANDA

void instruccion_MODR(int32_t *Registro1,int32_t *Registro2); //ANDA

void instruccion_DIVR(int32_t *Registro1,int32_t *Registro2); //ANDA

void instruccion_INCR(int32_t *Registro1); //ANDA

void instruccion_DECR(int32_t *Registro1);	//ANDA

void instruccion_COMP(int32_t *Registro1,int32_t *Registro2); //ANDA

void instruccion_CGEQ(int32_t *Registro1,int32_t *Registro2);	//ANDA

void instruccion_CLEQ(int32_t *Registro1,int32_t *Registro2); //ANDA

void instruccion_GOTO(int32_t *Registro);	//ANDA

void instruccion_JMPZ(int32_t Direccion);

void instruccion_JPNZ(int32_t Direccion);

void instruccion_INTE(int32_t Direccion);

void instruccion_MALC();

void instruccion_INNC();

void instruccion_OUTN();

void instruccion_CREA();

void instruccion_JOIN();

void instruccion_INNN();

void instruccion_FREE();

void instruccion_XXXX();

void instruccion_PUSH(int32_t *Registro,int32_t numero);

void instruccion_TAKE(int32_t *Registro,int32_t numero);

void instruccion_SHIF(int32_t *Registro,int32_t numero);

void instruccion_WAKE(int32_t direccion);

void cargar_registro_();

void cargar_registros_TCB();

#endif /* INSTRUCCIONES_CPU_H_ */
