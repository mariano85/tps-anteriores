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

int main() {

	inicializarMSP();

	conexionConKernelYCPU();

	//puts("LISTA MARCOS ANTES");
	//listarMarcos();


	/*crearSegmento(1234, 14);

	crearSegmento(5678, 25);

	crearSegmento(1234, 40);

	puts("PRIMER ESCRIBIR MEMORIA");
	escribirMemoria(1234, 1048576, "111111111122222222223333", 24);
	puts("SOLICITAR MEMORIA");
	solicitarMemoria(1234, 1048580, 23);*/

	//puts("TABLA SEGMENTOS ANTES");

	//tablaSegmentos();

	//validarEscrituraOLectura(1234, 45435435, 1003);

	/*int numeroSegmento, numeroPagina, offset;
	obtenerUbicacionLogica(1049344, &numeroSegmento, &numeroPagina, &offset);
	printf("numero segmento: %d    numero pagina: %d    offset: %d  \n", numeroSegmento, numeroPagina, offset);*/

	//printf("piso: %zu\n", generarDireccionLogica(1, 0, 4));


	//destruirSegmento(1234, 1);

	//puts("TABLA SEGMENTOS DESPUES");

	//tablaSegmentos();

	//puts("TABLA PAGINA");
	//tablaPaginas(1234);

	/*puts("PRIMER ESCRIBIR MEMORIA");
	escribirMemoria(1234, 1048576, "111111111122222222223333", 24);
	puts("SEGUNDO ESCRIBIR MEMORIA");
	escribirMemoria(5678, 0, "444444444455", 12);
	puts("TERCER ESCRIBIR MEMORIA");
	escribirMemoria(1234, 1049092, "6666666666666666", 16);*/


	/*escribirMemoria(1234, 0, "1111111111", 10);
	escribirMemoria(1234, 256, "2222222222", 10);
	escribirMemoria(1234, 1048576, "3333333333", 10);
	escribirMemoria(1234, 1048832, "4444444444", 10);
	escribirMemoria(1234, 1048088, "5555555555", 10);
	escribirMemoria(1234, 1049344, "6666666666", 10);
	escribirMemoria(5678, 0, "7777777777", 10);
	escribirMemoria(5678, 256, "8888888888", 10);
	escribirMemoria(5678, 512, "9999999999", 10);
	escribirMemoria(1234, 0, "0000000000", 10);*/













	/*puts("PRIMER ESCRIBIR MEMORIA");
	escribirMemoria(1234, 1048576, "1111111111", 10);
	escribirMemoria(1234, 1048576, "2222222222", 10);
	escribirMemoria(1234, 1048576, "3333333333", 10);
	escribirMemoria(1234, 1048576, "4444444444", 10);
	escribirMemoria(1234, 1048576, "5555555555", 10);
	escribirMemoria(1234, 1048576, "6666666666", 10);
	escribirMemoria(1234, 1048576, "7777777777", 10);
	escribirMemoria(1234, 1048576, "8888888888", 10);*/


	//validarLecturaOEscritura(1234, 4097, "hola", 12);

	puts("LISTA MARCOS despues");
	listarMarcos();


	/*int numeroSegmento, numeroPagina, offset;

	obtenerUbicacionLogica(12289, &numeroSegmento, &numeroPagina, &offset);

	printf("Numero segmento: %d    numero pagina: %d     offset: %d", numeroSegmento, numeroPagina, offset);*/



	return 0;


}
