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

int main(void) {
	configuracion = levantarArchivoDeConfiguracion();

	puts("////////IMPRIMO LOS VALORES//////////");
	printf ("aLGORITMO = %s\n", configuracion.sust_pags);
	printf ("tamaño msp = %d\n", configuracion.cantidad_memoria);
	printf ("tamaño swap = %d\n", configuracion.cantidad_swap);
	printf ("puerto = %d\n", configuracion.puerto);

	return 0;


}
