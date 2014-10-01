/*
 * instrucciones_CPU.c
 *
 *  Created on: 30/09/2014
 *      Author: utnso
 */

#include "cpu.h"
#include "instrucciones_CPU.h"

void LOAD(int *Registro,int Numero){



	*Registro = Numero;

}

void GETM(int *Registro1,int *Registro2){

	*Registro1 = *Registro2;

}

void ADDR(int *Registro1,int *Registro2){

	*Registro1 = *Registro1 + *Registro2;

}

void SUBR(int *Registro1,int *Registro2){


	*Registro1 = *Registro1 - *Registro2;


}

void MULR(int *Registro1,int *Registro2){


	*Registro1 = *Registro1 * *Registro2;


}

void MODR(int *Registro1,int *Registro2){


	*Registro1 = *Registro1 % *Registro2;


}


void DIVR(int *Registro1,int *Registro2){

	if(*Registro2 == 0){

		EFLAG = "ZERO_DIV";

	}else{

		*Registro1 = *Registro1 / *Registro2;

	}

}

void INCR(int *Registro1){


	*Registro1 = *Registro1 + 1;



}

void DECR(int *Registro1){


	*Registro1 = *Registro1 - 1;

}

void COMP(int *Registro1,int *Registro2){

	int aux_2;

	if(*Registro1 == *Registro2){

		aux_2 = 1;
	}else{

		aux_2 = 0 ;

	}

	*Registro1 = aux_2;

}

void CGEQ(int *Registro1,int *Registro2){

	int aux;

	if(*Registro1 >= *Registro2){

		aux = 1;
	}else{

		aux = 0;

	}

	*Registro1 = aux;
}

void CLEQ(int *Registro1,int *Registro2){

	int aux;

	if(*Registro1 <= *Registro2){

		aux = 1;
	}else{

		aux = 0;

	}

	*Registro1 = aux;
}

void GOTO(int *Registro){

	*PC = *Registro;

}

