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


	crearSegmento(1234, 778);

	crearSegmento(5678, 1589);

	puts("TABLA SEGMENTOS ANTES");

	tablaSegmentos();

	destruirSegmento(5678, 4097);

	puts("TABLA SEGMENTOS DESPUES");

	tablaSegmentos();

	//puts("TABLA PAGINA");
	//tablaPaginas(5678);

	puts("gola");

	return 0;


}
