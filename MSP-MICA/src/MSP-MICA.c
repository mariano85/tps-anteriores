/*
 ============================================================================
 Name        : MSP-MICA.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "msp.h"

int main() {

	inicializarMSP();




	//conexionConKernelYCPU();

	crearSegmento(1234, 10);

	crearSegmento(5678, 40);

	crearSegmento(1234, 40);

	crearSegmento(1234, 10);

	puts("tabla segmentos antes");
	tablaSegmentos();

	destruirSegmento(1234, 1048576);

	crearSegmento(1234, 20);

	puts("tabla segmentos despues");

	tablaSegmentos();

	escribirMemoria (1234, 0, "1111111111", 10);
	escribirMemoria (1234, 2097152, "2222222222", 10);

	escribirMemoria(5678, 0, "3333333333", 10);

	puts("solicito");
	puts(solicitarMemoria(1234, 0, 10));


	listarMarcos();

	puts("tabla paginas");
	tablaPaginas(1234);



	//escribirMemoria(1234, 0, "1111111111", 10);

	//escribirMemoria(5678, 0, "3333333333", 10);

	//int escribir = escribirMemoria(1234, 0, "4444444444555555555566666666667777777777", 40);

	//escribirMemoria(1234, 256, "5555555555", 10);

	//escribirMemoria(5678, 256, "3333333333", 10);

/*	printf("memoria restante: %d\n", memoriaRestante);
	printf("swap restante: %d\n", swapRestante);
	printf("retorno escribir: %d\n", escribir);
	listarMarcos();*/



/*

	int numeroSegmento, numeroPagina, offset;
	obtenerUbicacionLogica(1048842, &numeroSegmento, &numeroPagina, &offset);
	printf("segmento %d   numeropagina %d   off %d\n", numeroSegmento, numeroPagina, offset);
	uint32_t direccion = generarDireccionLogica(1, 3, 6);
	printf("direccion: %zu\n", direccion);*/






	/*escribirMemoria(5678, 768, "gggggggggg", 10);
	 *




	puts("listaDespues");

	listarMarcos();



	puts("solicitaaaas 5678");
	puts(solicitarMemoria(5678, 0, 30));
	puts("solicitar 1234");
	puts(solicitarMemoria(1234, 1048832, 30));*/

/*	destruirSegmento(5678, 0);
	destruirSegmento(1234, 0);
	destruirSegmento(1234, 1048576);

	puts("lista marcos despues de destruir");
	listarMarcos();
	printf("memoria restante: %d\n", memoriaRestante);
	printf("swap restante: %d\n", swapRestante);*/

	//moverPaginaDeSwapAMemoria(1234, 0, 0);
	//listarMarcos();


	/*//PRUEBA PARA AUMENTARPROGRAMCOUNTER
	uint32_t programCounterAnterior = 1048838;
	uint32_t nuevoProgramCounter;
	int bytesASumar = 4;
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
