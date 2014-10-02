/*
 * msp.c
 *
 *  Created on: 26/09/2014
 *      Author: utnso
 */
#include "msp.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <stdio.h>


#define CANTIDAD_MAX_PAGINAS_POR_SEGMENTO 3
#define CANTIDAD_MAX_PAGINAS_TOTAL 10

t_configuracion levantarArchivoDeConfiguracion()
{
 	int puerto;
	int tamanio;
	int swap;
	char* algoritmo;
	t_configuracion configuracion;


	char* path = "configuracion";


	//Abro el archivo de configuracion
	t_config *config;

	config = malloc(sizeof(t_config));

	config = config_create(path);

	//Compruebo si existe el archivo
	bool estaPuerto = config_has_property(config, "PUERTO");
	bool estaCantMemoria = config_has_property(config, "CANTIDAD_MEMORIA");
	bool estaCantSwap = config_has_property(config, "CANTIDAD_SWAP");
	bool estaSust = config_has_property(config, "SUST_PAGS");

	if(!estaPuerto || !estaCantMemoria || !estaCantSwap || !estaSust)
	{
		printf("Error en el archivo de configuracion\n");
		log_trace(logs, "Error en el archivo de configuracion");
		abort();

	}


	//Pido los valores de configuracion y los vuelco en una estructura
	algoritmo = config_get_string_value(config, "SUST_PAGS");
	puerto = config_get_int_value(config, "PUERTO");
	tamanio = config_get_int_value(config, "CANTIDAD_MEMORIA");
	swap = config_get_int_value(config, "CANTIDAD_SWAP");

	configuracion.sust_pags = algoritmo;
	configuracion.puerto = puerto;
	configuracion.cantidad_memoria = tamanio;
	configuracion.cantidad_swap = swap;

	free(config);

	//Devuelvo la estructura cuyos campos son los parametros de configuracion
	return configuracion;

}

void crearTablaDeMarcos()
{
	//Creo la tabla de marcos. En realidad es una lista, porque el tamaño lo obtengo
	//en tiempo de ejecucion. La cantidad de filas es igual a la cantidad maxima de marcos
	//que puede tener la memoria teniendo en cuenta el tamaño seteado.

	listaMarcos = list_create();

	long i;
	for (i = 1; i <= CANTIDAD_MAX_PAGINAS_TOTAL; i++)
	{
		nodo_marcos* nodo;
		nodo = malloc(sizeof(nodo_marcos));

		nodo -> nro_segmento = 0;
		nodo -> nro_pagina = 0;
		nodo -> pid = 0;
		nodo -> nro_marco = i;
		list_add(listaMarcos, nodo);
	}

}

void listarMarcos()
{
	int i;
	for (i=0; i<CANTIDAD_MAX_PAGINAS_TOTAL; i++)
	{
		nodo_marcos *nodoMarco;
		nodoMarco = malloc(sizeof(nodoMarco));
		nodoMarco = list_get(listaMarcos, i);

		printf("N° marco: %ld     PID: %d    N° segmento: %d    N° pagina: %d\n", nodoMarco->nro_marco, nodoMarco->pid, nodoMarco->nro_segmento, nodoMarco->nro_pagina);
		free(nodoMarco);
	}
}

void inicializarMSP()
{
	//Abro el archivo de log
	logs = log_create("logMSP", "MSP", 0, LOG_LEVEL_TRACE);
	//Registro el inicio de ejecucion de la MSP
	log_trace(logs, "MSP inicio su ejecucion");

	//Obtengo los parametros de configuracion del archivo de configuracion.
	configuracion = levantarArchivoDeConfiguracion();

	//Creo la tabla de marcos
	crearTablaDeMarcos();

	//Creo la lista se segmentos
	listaSegmentos = list_create();


	//CONECTAR CON KERNEL
	//ABRIR CONEXIONES CON CPU

	//Reservo el gran bloque de memoria que va a actuar como MP
	//ptoMP = malloc(configuracion.cantidad_memoria);


	//free(ptoMP);
}

void agregarSegmentoALista(int cantidadDePaginas, int pid)
{
	nodo_segmento *nodoSegmento;
	nodoSegmento = malloc(sizeof(nodo_segmento));

	int cantidadActualSegmentos = list_size(listaSegmentos);

	t_list *listaPaginas;
	listaPaginas = crearListaPaginas(cantidadDePaginas);

	nodoSegmento->numeroSegmento = cantidadActualSegmentos;
	nodoSegmento->pid = pid;
	nodoSegmento->listaPaginas = listaPaginas;

	list_add(listaSegmentos, nodoSegmento);


	printf("Nro seg: %d    PID: %d\n", nodoSegmento->numeroSegmento, nodoSegmento->pid);

	int i;
	for(i=0; i<cantidadDePaginas; i++)
	{
		nodo_paginas *nodoPagina;
		nodoPagina = list_get(listaPaginas, i);

		printf("		 Nro pag: %ld      presencia: %ld\n",  nodoPagina->nro_pagina, nodoPagina->presencia);

	}
}

void crearSegmento(int pid, long tamanio)
{

	int contadorSegmentos;
	int cantidadTotalDePaginas;

	cantidadTotalDePaginas = tamanio / 256;

	int cantidadDeSegmentosEnteros = cantidadTotalDePaginas / CANTIDAD_MAX_PAGINAS_POR_SEGMENTO;

	if (cantidadDeSegmentosEnteros > 0)
	{
		for (contadorSegmentos=0; contadorSegmentos<cantidadDeSegmentosEnteros; contadorSegmentos++)
		{
			agregarSegmentoALista(CANTIDAD_MAX_PAGINAS_POR_SEGMENTO, pid);
		}
	}
	else
	{
		agregarSegmentoALista(cantidadTotalDePaginas, pid);
	}

	int paginasQueFaltan = tamanio % 256;

	if (paginasQueFaltan > 0)
	{
		agregarSegmentoALista(paginasQueFaltan, pid);
	}
}

t_list* crearListaPaginas(int cantidadDePaginas)
{
	t_list* listaPaginas;

	listaPaginas = list_create();

	int i;

	for (i = 0; i<cantidadDePaginas; i++)
	{
		nodo_paginas* nodoPagina = malloc(sizeof(nodo_paginas));
		nodoPagina->nro_pagina = i;
		nodoPagina->presencia = 0;

		list_add(listaPaginas, nodoPagina);
	}

	return listaPaginas;
}

void tablaSegmentos()
{
	int cantidadActualSegmentos = list_size(listaSegmentos);
	int i;
	for (i=0; i<cantidadActualSegmentos; i++)
	{
		nodo_segmento *nodoSegmento;
		nodoSegmento = list_get(listaSegmentos, i);
		printf("PID: %d       N° segmento: %d\n", nodoSegmento->pid, nodoSegmento->numeroSegmento);
	}

}

void tablaPaginas(int pid)
{
	bool _pidCorresponde(nodo_segmento *p) {
			return (p->pid == pid);
		}

	t_list *listaFiltrada;
	t_list *listaPaginas;

	listaFiltrada = list_filter(listaSegmentos, (void*)_pidCorresponde);


	int cantidadSegmentosFiltrados = list_size(listaFiltrada);

	if(cantidadSegmentosFiltrados == 0)
	{
		log_trace(logs, "No se encontro el pid solicitado");
		puts("Error, no se encontro el pid solicitado.");
		abort();
	}

	int sizeListaPaginas;

	int i;
	int j;

	for(i = 0; i<cantidadSegmentosFiltrados; i++)
	{
		nodo_segmento *nodoSegmento;
		nodoSegmento = list_get(listaFiltrada, i);

		listaPaginas = nodoSegmento->listaPaginas;
		sizeListaPaginas = list_size(listaPaginas);

		for (j=0; j<sizeListaPaginas; j++)
		{
			nodo_paginas *nodoPagina;
			nodoPagina = list_get(listaPaginas, j);
			printf("Nro segmento :%d     PID: %d    N° pagina: %ld     Presencia: %ld\n", nodoSegmento->numeroSegmento, nodoSegmento->pid, nodoPagina->nro_pagina, nodoPagina->presencia);

		}

	}

}
