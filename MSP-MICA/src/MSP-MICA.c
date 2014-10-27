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
#include <unistd.h>

int main() {

	inicializarMSP();

	//conexionConKernelYCPU();

	crearSegmento(1234, 20);

	crearSegmento(5678, 40);

	crearSegmento(1234, 40);

	escribirMemoria(1234, 0, "1111111111", 10);

	escribirMemoria(5678, 0, "3333333333", 10);
	escribirMemoria(5678, 256, "4444444444", 10);
	escribirMemoria(1234, 1048576, "5555555555", 10);

	escribirMemoria(1234, 256, "2222222222", 10);

	escribirMemoria(1234, 1048832, "6666666666", 10);
	escribirMemoria(1234, 1049088, "7777777777", 10);
	//escribirMemoria(1234, 1049344, "8888888888", 10);
/*	escribirMemoria(1234, 0, "1111111111", 10);




	escribirMemoria(5678, 512, "9999999999", 10);
	escribirMemoria(5678, 768, "aaaaaaaaaa", 10);
*/

	//elegirVictimaSegunFIFO();




	//int numeroSegmento, numeroPagina, offset;
	//obtenerUbicacionLogica()
/*
	uint32_t direccion = generarDireccionLogica(1, 3, 6);
	printf("direccion: %zu\n", direccion);*/







	puts("lista antes");
	listarMarcos();

	escribirMemoria(5678, 0, "aaaaaaaaaabbbbbbbbbbcccccccccc", 30);

	puts("listaDespues");

	listarMarcos();

	//tablaPaginas(1234);
//	tablaPaginas(5678);

	puts("solicitaaaas");
	puts(solicitarMemoria(5678, 0, 30));














	/*PRUEBA PARA AUMENTARPROGRAMCOUNTER
	 * uint32_t programCounterAnterior = 1048833;
	uint32_t nuevoProgramCounter;
	int bytesASumar = 11;
	int numeroSegmento, numeroPagina, offset, numeroSegmentoViejo, numeroPaginaViejo, offsetViejo;

	nuevoProgramCounter = aumentarProgramCounter(programCounterAnterior, bytesASumar);

	obtenerUbicacionLogica(programCounterAnterior, &numeroSegmentoViejo, &numeroPaginaViejo, &offsetViejo);

	printf("VIEJO PC: Numero segmento: %d    numero pagina: %d     offset: %d\n", numeroSegmentoViejo, numeroPaginaViejo, offsetViejo);

	obtenerUbicacionLogica(nuevoProgramCounter, &numeroSegmento, &numeroPagina, &offset);

	printf("NUEVO PC: Numero segmento: %d    numero pagina: %d     offset: %d\n", numeroSegmento, numeroPagina, offset);

	uint32_t direccion = generarDireccionLogica(1, 1, 12);

	printf("direccion: %zu\n", direccion);

	obtenerUbicacionLogica(direccion, &numeroSegmento, &numeroPagina, &offset);

	printf("OTRO PC: Numero segmento: %d    numero pagina: %d     offset: %d\n", numeroSegmento, numeroPagina, offset);*/




	return 0;


}
