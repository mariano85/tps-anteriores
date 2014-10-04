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
#include "msp.h"

int main() {

	inicializarMSP();

	//listarMarcos();


	/*crearSegmento(1234, 778);

	crearSegmento(5678, 258);

	crearSegmento(1234, 778);

	puts("TABLA SEGMENTOS ANTES");

	tablaSegmentos();

	destruirSegmento(1234, 1);

	puts("TABLA SEGMENTOS DESPUES");

	tablaSegmentos();

	puts("TABLA PAGINA");
	tablaPaginas(1234);*/

	int numeroSegmento, numeroPagina, offset;

	obtenerUbicacionLogica(12289, &numeroSegmento, &numeroPagina, &offset);

	printf("Numero segmento: %d    numero pagina: %d     offset: %d", numeroSegmento, numeroPagina, offset);


	puts("gola");

	return 0;


}
