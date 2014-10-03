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


	crearSegmento(1234, 513);

	crearSegmento(5678, 1537);

	//puts("TABLA SEGMENTOS");

	//tablaSegmentos();

	//puts("TABLA PAGINA");
	//tablaPaginas(5678);

	puts("gola");

	return 0;


}
