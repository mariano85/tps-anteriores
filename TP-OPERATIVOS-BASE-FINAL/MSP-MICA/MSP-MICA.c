/*
 ============================================================================
 Name        : MSP-MICA.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "msp.h"
#include <string.h>
#include <stddef.h>


char* getBytesFromFile(FILE* entrada, size_t *tam_archivo);

int main(int argc, char *argv[]) {

	inicializarMSP();

//	FILE *entrada;


	/*	char * NOM_ARCHIVO = argv[1];

			if ((entrada = fopen(NOM_ARCHIVO, "r")) == NULL){
				perror(NOM_ARCHIVO);
				return EXIT_FAILURE;
			}

			size_t cantBytes = 0;
			char *literal = getBytesFromFile(entrada, &cantBytes);


			puts("Hola aca te mando la instruccion");
			puts(literal);

			fclose(entrada);

			puts("eso es todo el archivo");
			crearSegmento(1234, 20);
			crearSegmento(1234, 100);*/
	//		crearSegmento(1234,30);


//	escribirMemoria(1234,1048576,literal,100);

	listarMarcos();

//	puts("Solicitooooooo");
//	puts(solicitarMemoria(1234, 1049096, 4));

	conexionConKernelYCPU();





	//int numero;

	/*char* buffer = malloc(4);
	memset(buffer,0,4);

	buffer = solicitarMemoria(1234,1048581,1);

	memcpy(&numero,buffer,sizeof(int));

	log_info(logs,"el valor es %d",numero);*/


//	free(literal);
		return 0;



}


char* getBytesFromFile(FILE* entrada, size_t *tam_archivo){
	fseek(entrada, 0L, SEEK_END);
	*tam_archivo = ftell(entrada);
	char * literal = (char*) calloc(1, *tam_archivo);
	fseek(entrada, 0L, 0L);

	fread(literal, sizeof(char), *tam_archivo, entrada);
	//fgets(literal, *tam_archivo, entrada);
	return literal;

}


