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

	//puts("LISTA MARCOS ANTES");
	//listarMarcos();


	printf("Direccion base: %d \n", crearSegmento(1234, 778));

	printf("Direccion base: %d\n", crearSegmento(5678, 258));

	printf("Direccion base: %d\n", crearSegmento(1234, 778));

	//puts("TABLA SEGMENTOS ANTES");

	//tablaSegmentos();

	//validarEscrituraOLectura(1234, 45435435, 1003);

	int numeroSegmento, numeroPagina, offset;
	obtenerUbicacionLogica(45435435, &numeroSegmento, &numeroPagina, &offset);
	printf("numero segmento: %d    numero pagina: %d    offset: %d  \n", numeroSegmento, numeroPagina, offset);

	printf("piso: %zu\n", generarDireccionLogica(3, 677, 254));


	//destruirSegmento(1234, 1);

	//puts("TABLA SEGMENTOS DESPUES");

	//tablaSegmentos();

	//puts("TABLA PAGINA");
	//tablaPaginas(1234);

	/*puts("PRIMER ESCRIBIR MEMORIA");
	escribirMemoria(1234, 0, "hola", 780);
	puts("SEGUNDO ESCRIBIR MEMORIA");
	unsigned int direccion = (unsigned int)4194304000;
	escribirMemoria(5678, direccion, "hola", 5);
	puts("TERCER ESCRIBIR MEMORIA");
	escribirMemoria(1234, 4096, "hola", 768);*/

	//validarLecturaOEscritura(1234, 4097, "hola", 12);

	puts("LISTA MARCOS despues");
	listarMarcos();


	/*int numeroSegmento, numeroPagina, offset;

	obtenerUbicacionLogica(12289, &numeroSegmento, &numeroPagina, &offset);

	printf("Numero segmento: %d    numero pagina: %d     offset: %d", numeroSegmento, numeroPagina, offset);*/



	return 0;


}
