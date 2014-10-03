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
#include <stdint.h>


#define CANTIDAD_MAX_PAGINAS_POR_SEGMENTO 3
#define CANTIDAD_MAX_PAGINAS_TOTAL 100



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
	tablaMarcos = calloc(CANTIDAD_MAX_PAGINAS_TOTAL, sizeof(t_marco));

	if (tablaMarcos == NULL)
	{
		puts("Error, no se pudo crear la tabla de marcos");
		log_trace(logs, "ERROR: no se pudo crear la tabla de marcos.");
		abort();
	}

	int i;
	for (i=0; i<CANTIDAD_MAX_PAGINAS_TOTAL; i++)
	{
		tablaMarcos[i].nro_marco = i;
		tablaMarcos[i].dirFisica = NULL;
	}


}

void listarMarcos()
{
	int i;
	for (i=0; i<CANTIDAD_MAX_PAGINAS_TOTAL; i++)
	{
		printf("Numero marco: %d     PID: %d     Numero segmento: %d     Numero pagina: %d\n", tablaMarcos[i].nro_marco, tablaMarcos[i].pid, tablaMarcos[i].nro_segmento, tablaMarcos[i].nro_pagina);
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


	memoriaRestante = configuracion.cantidad_memoria;
	swapRestante = configuracion.cantidad_swap;

	//Creo la tabla de marcos
	crearTablaDeMarcos();

	//Creo la lista se segmentos
	listaSegmentos = list_create();


	//CONECTAR CON KERNEL
	//ABRIR CONEXIONES CON CPU

	//Reservo el gran bloque de memoria que va a actuar como MP
	ptoMP = malloc(configuracion.cantidad_memoria);
	if (ptoMP == NULL)
	{
		log_trace(logs, "No es posible alocar memoria principal");
		abort();
	}


	//free(ptoMP);
}

void agregarSegmentoALista(int cantidadDePaginas, int pid, int numeroSegmento)
{
	nodo_segmento *nodoSegmento;
	nodoSegmento = malloc(sizeof(nodo_segmento));

	t_list *listaPaginas;
	listaPaginas = crearListaPaginas(cantidadDePaginas);

	nodoSegmento->numeroSegmento = numeroSegmento;
	nodoSegmento->pid = pid;
	nodoSegmento->listaPaginas = listaPaginas;

	list_add(listaSegmentos, nodoSegmento);


	printf("Nro seg: %d    PID: %d\n", nodoSegmento->numeroSegmento, nodoSegmento->pid);

	uint32_t direccion = generarDireccionLogica(nodoSegmento->numeroSegmento, 1, 0);

	int i;
	for(i=0; i<cantidadDePaginas; i++)
	{
		nodo_paginas *nodoPagina;
		nodoPagina = list_get(listaPaginas, i);

		printf("		 Nro pag: %d      presencia: %d       direccion: %zu\n",  nodoPagina->nro_pagina, nodoPagina->presencia, direccion);

	}

}

void crearSegmento(int pid, long tamanio)
{

	int contadorSegmentos = 0;
	int cantidadTotalDePaginas;

	if (memoriaRestante < tamanio && swapRestante < tamanio)
	{
		printf("Error, no hay memoria ni espacio de swap suficiente.\n");
	}

	if (tamanio % 256 == 0)
	{
		cantidadTotalDePaginas = tamanio / 256;
	}
	else
	{
		cantidadTotalDePaginas = tamanio / 256 + 1;
	}

	int cantidadDeSegmentosEnteros = cantidadTotalDePaginas / CANTIDAD_MAX_PAGINAS_POR_SEGMENTO;

	if (cantidadDeSegmentosEnteros > 0)
	{
		for (contadorSegmentos=0; contadorSegmentos<cantidadDeSegmentosEnteros; contadorSegmentos++)
		{
			agregarSegmentoALista(CANTIDAD_MAX_PAGINAS_POR_SEGMENTO, pid, contadorSegmentos);
		}
	}
	else
	{
		agregarSegmentoALista(cantidadTotalDePaginas, pid, 0);
		contadorSegmentos = 1;
	}

	int paginasQueFaltan = cantidadTotalDePaginas - cantidadDeSegmentosEnteros * CANTIDAD_MAX_PAGINAS_POR_SEGMENTO;

	if (paginasQueFaltan > 0)
	{
		agregarSegmentoALista(paginasQueFaltan, pid, contadorSegmentos);
	}
}

void destruirSegmento(int pid, uint32_t base)
{
	int numeroSegmento, numeroPagina, offset;

	obtenerUbicacionLogica(base, &numeroSegmento, &numeroPagina, &offset);

	bool _pidYSegmentoCorresponde(nodo_segmento *p) {
		return (p->pid == pid && p->numeroSegmento == numeroSegmento);
	}

	nodo_segmento * nodo;


	if (!list_any_satisfy(listaSegmentos, (void*)_pidYSegmentoCorresponde))
	{
		printf("Error, pid invalido\n");
	}
	else
	{
		nodo = list_remove_by_condition(listaSegmentos, (void*)_pidYSegmentoCorresponde);
		free(nodo);
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
			printf("Nro segmento :%d     PID: %d    N° pagina: %d     Presencia: %d\n", nodoSegmento->numeroSegmento, nodoSegmento->pid, nodoPagina->nro_pagina, nodoPagina->presencia);

		}

	}

}

uint32_t generarDireccionLogica(int numeroSegmento, int numeroPagina, int offset)
{
	uint32_t direccion = offset;
	direccion = direccion << 12;
	direccion = direccion | numeroPagina;
	direccion = direccion << 12;
	direccion = direccion | numeroSegmento;

	return direccion;
}

void obtenerUbicacionLogica(uint32_t direccion, int *numeroSegmento, int *numeroPagina, int *offset)
{
	*numeroSegmento = direccion & 0xFFF;
	*numeroPagina = (direccion >> 12) & 0xFFF;
	*offset = (direccion >> 24) & 0xFF;
}
