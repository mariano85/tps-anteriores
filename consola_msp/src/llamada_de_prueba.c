/*
 ============================================================================
 Name        : llamada_de_prueba.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "consola/consola_msp.h"
#include <pthread.h>

int main(void){


/* Copiar el codigo en la inicializacion de la MSP, habiendo copiado la carpeta Consola al src de la MSP. */

	pthread_t hilo_consola_1;
	if(pthread_create(&hilo_consola_1, NULL, (void*) consola_msp(), NULL)!=0){
		puts("No se ha podido crear el proceso consola de la MSP.");
		/* ACA VA UN LOG DE ERROR */
	}


	return 0;
}
