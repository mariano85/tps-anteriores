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

	//listarMarcos();


	printf("%d \n", crearSegmento(1234, 778));

	printf("%d\n", crearSegmento(5678, 258));

	printf("%d\n", crearSegmento(1234, 778));

	puts("TABLA SEGMENTOS ANTES");

	tablaSegmentos();

	validarEscrituraOLectura(1234, 4278202368, "hola", 128);

	int numeroSegmento, numeroPagina, offset;
	obtenerUbicacionLogica(4278202368, &numeroSegmento, &numeroPagina, &offset);
	printf("numero segmento: %d    numero pagina: %d    offset: %d  \n", numeroSegmento, numeroPagina, offset);

	printf("pisobis: %zu\n", generarDireccionLogica(0, 3, 255));


	//destruirSegmento(1234, 1);

	//puts("TABLA SEGMENTOS DESPUES");

	//tablaSegmentos();

	//puts("TABLA PAGINA");
	//tablaPaginas(1234);/*

	//puts("ESCRIBIR MEMEMEMME");

	//validarLecturaOEscritura(1234, 4097, "hola", 12);

	puts("LISTA MARCOS");
	//listarMarcos();


	/*int numeroSegmento, numeroPagina, offset;

	obtenerUbicacionLogica(12289, &numeroSegmento, &numeroPagina, &offset);

	printf("Numero segmento: %d    numero pagina: %d     offset: %d", numeroSegmento, numeroPagina, offset);*/


	puts("gola");

	return 0;


}
