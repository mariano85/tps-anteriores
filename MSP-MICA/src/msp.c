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


#define CANTIDAD_MAX_SEGMENTOS_POR_PID 3
#define CANTIDAD_MAX_PAGINAS_POR_SEGMENTO 5


t_configuracion levantarArchivoDeConfiguracion()
{
 	int puerto;
	int tamanio;
	int swap;
	char* algoritmo;
	t_configuracion configuracion;


	char* path = "configuracion";

	t_config *config;

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

	algoritmo = config_get_string_value(config, "SUST_PAGS");
	puerto = config_get_int_value(config, "PUERTO");
	tamanio = config_get_int_value(config, "CANTIDAD_MEMORIA");
	swap = config_get_int_value(config, "CANTIDAD_SWAP");

	configuracion.sust_pags = algoritmo;
	configuracion.puerto = puerto;
	configuracion.cantidad_memoria = tamanio;
	configuracion.cantidad_swap = swap;

	return configuracion;
}

void crearTablaDeMarcos()
{
	if (configuracion.cantidad_memoria % 256 == 0)
	{
		cantidadMarcos = configuracion.cantidad_memoria / 256;
	}
	else
	{
		//Si la división no es exacta, necesito un marco más para alojar lo que queda de la memoria.
		cantidadMarcos = configuracion.cantidad_memoria / 256 + 1;
	}

	//Aloco espacio de memoria para una tabla que va tener toda la información de los marcos de memoria.
	//calloc recibe dos parámetros por separado, la cantidad de espacios que necesito, y el tamaño de cada
	//uno de esos espacios, y además, settea cada espacio con el caracter nulo.
	tablaMarcos = calloc(cantidadMarcos, sizeof(t_marco));

	if (tablaMarcos == NULL)
	{
		puts("Error, no se pudo crear la tabla de marcos");
		log_trace(logs, "ERROR: no se pudo crear la tabla de marcos.");
		abort();
	}

	int i;
	for (i=0; i<cantidadMarcos; i++)
	{

		tablaMarcos[i].nro_marco = i;

		tablaMarcos[i].dirFisica = memoriaPrincipal + i*256;

		//Cuando recién se crea la tabla, todos los marcos están libres.
		tablaMarcos[i].libre = 1;
	}
}

void listarMarcos()
{
	int i;
	for (i=0; i<cantidadMarcos; i++)
	{
		printf("Numero marco: %d     PID: %d     Numero segmento: %d     Numero pagina: %d\n", tablaMarcos[i].nro_marco, tablaMarcos[i].pid, tablaMarcos[i].nro_segmento, tablaMarcos[i].nro_pagina);
	}
}

void inicializarMSP()
{
	logs = log_create("logMSP", "MSP", 0, LOG_LEVEL_TRACE);
	log_trace(logs, "MSP inicio su ejecucion");

	configuracion = levantarArchivoDeConfiguracion();

	//Estas variables las uso para, cada vez que asigno un marco o swappeo una pagina, voy restando del
	//espacio total. Cuando alguna de estas dos variables llegue a 0, significa que no hay mas espacio.
	memoriaRestante = configuracion.cantidad_memoria;
	swapRestante = configuracion.cantidad_swap;

	memoriaPrincipal = malloc(configuracion.cantidad_memoria);
	if(memoriaPrincipal == NULL)
	{
		log_error(logs, "Error, no se pudo alocar espacio para la memoria principal.");
		abort();
	}

	crearTablaDeMarcos();

	listaSegmentos = list_create();


	//CONECTAR CON KERNEL
	//ABRIR CONEXIONES CON CPU




	log_trace(logs, "MSP inicio su ejecucion. Tamaño memoria: %d", configuracion.cantidad_memoria, "Tamaño swap: %d", configuracion.cantidad_swap);

}

uint32_t agregarSegmentoALista(int cantidadDePaginas, int pid, int cantidadSegmentosDeEstePid)
{
	nodo_segmento *nodoSegmento;
	nodoSegmento = malloc(sizeof(nodo_segmento));

	t_list *listaPaginas;
	listaPaginas = crearListaPaginas(cantidadDePaginas);

	nodoSegmento->numeroSegmento = cantidadSegmentosDeEstePid;
	nodoSegmento->pid = pid;
	nodoSegmento->listaPaginas = listaPaginas;

	list_add(listaSegmentos, nodoSegmento);


	//Esta es una impresion de prueba.
	printf("Nro seg: %d    PID: %d\n", nodoSegmento->numeroSegmento, nodoSegmento->pid);
	int i;
	for(i=0; i<cantidadDePaginas; i++)
	{
		nodo_paginas *nodoPagina;
		nodoPagina = list_get(listaPaginas, i);
		uint32_t direccion = generarDireccionLogica(nodoSegmento->numeroSegmento, nodoPagina->nro_pagina, 0);

		printf("		 Nro pag: %d      presencia: %d       direccion: %zu\n",  nodoPagina->nro_pagina, nodoPagina->presencia, direccion);

	}

	uint32_t direccionBase = generarDireccionLogica(nodoSegmento->numeroSegmento, 0, 0);

	return direccionBase;

}

uint32_t crearSegmento(int pid, long tamanio)
{
	int cantidadTotalDePaginas;

	if (memoriaRestante < tamanio && swapRestante < tamanio)
	{
		printf("Error, no hay memoria ni espacio de swap suficiente.\n");
		log_trace(logs, "Error, no hay memoria ni espacio de swap suficiente.");
		abort();
	}

	bool _pidCorresponde(nodo_segmento *p) {
			return (p->pid == pid);
		}
	t_list *listaDeSegmentosDeEstePid = list_filter(listaSegmentos, (void*)_pidCorresponde);
	int cantidadSegmentosDeEstePid = list_size(listaDeSegmentosDeEstePid);
	if (cantidadSegmentosDeEstePid == CANTIDAD_MAX_SEGMENTOS_POR_PID)
	{
		printf("Error, ya hay 4096 segmentos para este PID, no se puede agregar nignuno más.");
		log_trace(logs, "Error, ya hay 4096 segmentos para este PID, no se puede agregar nignuno más.");
		return -1;
	}

	//Si el resto de la división entre el tamaño y 256 (que es el tamaño máximo de la pagina) da 0...
	if (tamanio % 256 == 0)
	{
		//...entonces la cantiadad de páginas es el resultado de la división, si no...
		cantidadTotalDePaginas = tamanio / 256;
	}
	else
	{
		//...la cantidad total de páginas es el resultado de la división más 1, para agregar en una página lo que
		//queda del tamaño, por supuesto, esta última página no estará completa.
		cantidadTotalDePaginas = tamanio / 256 + 1;
	}

	if (cantidadTotalDePaginas > CANTIDAD_MAX_PAGINAS_POR_SEGMENTO)
	{
		log_error(logs, "Error, no se puede crear el segmento porque excede el tamaño máximo.");
		puts("Error, no se puede crear el segmento porque excede el tamaño máximo.");
		return -1;
	}

	uint32_t direccionBase = agregarSegmentoALista(cantidadTotalDePaginas, pid, cantidadSegmentosDeEstePid);

	bool _ordenar(nodo_segmento *seg1, nodo_segmento *seg2)	{
		return (seg1->pid <= seg2->pid);
	}
	list_sort(listaSegmentos,(void*)_ordenar);


	return direccionBase;

}

void destruirSegmento(int pid, uint32_t base)
{
	int numeroSegmento, numeroPagina, offset;

	obtenerUbicacionLogica(base, &numeroSegmento, &numeroPagina, &offset);

	if (numeroPagina != 0 || offset != 0)
	{
		log_error(logs, "Error, la dirección proporcionada no es una dirección base.");
		puts("Error, la dirección proporcionada no es una dirección base.");
		return;
	}

	bool _pidYSegmentoCorresponde(nodo_segmento *p) {
		return (p->pid == pid && p->numeroSegmento == numeroSegmento);
	}

	nodo_segmento * nodo;

	if (!list_any_satisfy(listaSegmentos, (void*)_pidYSegmentoCorresponde))
	{
		log_error(logs, "Error, PID y/o segmento inválidos.");
		printf("Error, pid y/o segmento invalidos\n");
		return;
	}
	else
	{
		nodo = list_remove_by_condition(listaSegmentos, (void*)_pidYSegmentoCorresponde);
		free(nodo->listaPaginas);
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
		nodoPagina->presencia = -1;

		list_add(listaPaginas, nodoPagina);
	}

	return listaPaginas;
}

void tablaSegmentos()
{
	void _imprimirSegmento(nodo_segmento *nodoSegmento)
	{
		printf("PID: %d       N° segmento: %d\n", nodoSegmento->pid, nodoSegmento->numeroSegmento);
	}

	list_iterate(listaSegmentos, (void*)_imprimirSegmento);

}

void tablaPaginas(int pid)
{
	t_list *listaFiltrada = filtrarListaSegmentosPorPid(listaSegmentos, pid);
	t_list *listaPaginas;


	if(list_is_empty(listaFiltrada))
	{
		log_trace(logs, "No se encontró el pid solicitado");
		puts("Error, no se encontro el pid solicitado.");
		abort();
	}

	int sizeListaPaginas;
	int cantidadSegmentosFiltrados = list_size(listaFiltrada);
	int i, j;

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

t_list* filtrarListaSegmentosPorPid(t_list* listaSegmentos, int pid)
{
	bool _pidCorresponde(nodo_segmento *p) {
		return (p->pid == pid);
	}

	t_list *listaFiltrada;

	listaFiltrada = list_filter(listaSegmentos, (void*)_pidCorresponde);

	return listaFiltrada;
}

bool validarEscrituraOLectura(int pid, uint32_t direccionLogica, void* bytesAEscribir, int tamanio)
{
	if (tamanio > memoriaRestante)
	{
		log_error(logs, "Error, no hay espacio suficiente en la memoria.");
		puts("Error, no hay espacio suficiente en la memoria.");
		return false;
	}

	t_list* listaFiltradaPorPid = filtrarListaSegmentosPorPid(listaSegmentos, pid);

	if (list_is_empty(listaFiltradaPorPid))
	{
		log_error(logs, "Error, el pid ingresado no existe.");
		puts("Error, el pid ingresado no existe.");
		return false;
	}

	int numeroSegmento, numeroPagina, offset;
	obtenerUbicacionLogica(direccionLogica, &numeroSegmento, &numeroPagina, &offset);
	if (offset > 255)
	{
		log_error(logs, "Error, la dirección ingresada es inválida.");
		puts("Error, la dirección ingresada es inválida.");
		return false;
	}

	//Ahora de la lista de los segmentos correspondientes a este pid, quiero el segmento
	//que me dice la dirección lógica. La función find me devuelve el primer resultado que encuentra,
	//pero en este caso eso está bien porque hay un sólo segmento de número numeroSegmento en el espacio de
	//direcciones del proceso pid.
	nodo_segmento *nodoSegmento;
	bool _segmentoCorresponde(nodo_segmento *p){
		return(p->numeroSegmento == numeroSegmento);
	}
	nodoSegmento = list_find(listaFiltradaPorPid, (void*)_segmentoCorresponde);

	if (nodoSegmento == NULL)
	{
		log_error(logs, "Error, la dirección ingresada es inválida.");
		puts("Error, la dirección ingresada es inválida.");
		return false;
	}

	t_list* listaPaginas = nodoSegmento->listaPaginas;

	nodo_paginas *nodoPagina;
	bool _paginaCorresponde(nodo_paginas *p){
		return(p->nro_pagina == numeroPagina);
	}
	nodoPagina = list_find(listaPaginas, (void*)_paginaCorresponde);

	if (nodoPagina == NULL)
	{
		log_error(logs, "Error, la dirección ingresada es inválida.");
		puts("Error, la dirección ingresada es inválida.");
		return false;
	}

	//Tengo que transformar el tamanio en paginas, para ver si el segmneto tiene esa cantidad de paginas
	int faltaParaCompletarPagina = 256 - offset;
	int quedaDelTamanio = tamanio - faltaParaCompletarPagina;
	int cantidadPaginasEnterasQueQuedan = quedaDelTamanio / 256;
	//Me fijo si necesito una pagina para alojar el remanente
	int cantidadPaginasQueOcupaTamanio;
	if (quedaDelTamanio % 256 != 0)
	{
		cantidadPaginasQueOcupaTamanio = cantidadPaginasEnterasQueQuedan + 1;
	}
	else
	{
		cantidadPaginasQueOcupaTamanio = cantidadPaginasEnterasQueQuedan;
	}
	//Ahora, cantidadPaginasQueOcupaTamanio representa, a partir de la pagina dedse donde empiezo a escribir (y sin contarla) las páginas que
	//tendría que tener el segmento para que tamanio entre
	int cantidadPaginasSegmento = list_size(listaPaginas);
	if ((nodoPagina->nro_pagina + cantidadPaginasQueOcupaTamanio) > (cantidadPaginasSegmento -1))
	{
		log_error(logs, "Error, violación de segmento.");
		puts("Error, violacion de segmento.");
		return false;
	}

	printf("pagina final: %d\n", nodoPagina->nro_pagina + cantidadPaginasQueOcupaTamanio);
	printf("termino\n");
	return true;


}


/*void escribirMemoria(int pid, uint32_t direccionLogica, void* bytesAEscribir, int tamanio)
{



	//Ahora, en nodoPagina tengo, en el campo presencia, lo siguiente:
		//-1 indica que todavía no le asigné ningún marco
		//-2 indica que está en swap (esto por ahora lo ignoro)
		//otro número indica el número de marco
	//Si presencia es -1, busco un marco libre y se lo asigno a la página
	if (nodoPagina->presencia == -1)
	{
		//Esta es la dirección donde empieza el bloque de memoria al que apunta el marco
		void *direccionComienzoBloque;
		direccionComienzoBloque = buscarYAsignarMarcoLibre(pid, nodoSegmento, nodoPagina);
		//A la dirección, le tengo que agregar el offset, para posicionarme en la dirección en la cual
		//voy a empezar a escribir la memoria
		void *direccionDestino = direccionComienzoBloque + offset;
		//Ahora sí, escribo en esa dirección de memoria el contenido del buffer
		memcpy(direccionDestino, bytesAEscribir, tamanio);
	}

	printf("Numero segmento: %d    pid: %d     \n", nodoSegmento->numeroSegmento, nodoSegmento->pid);


}
*/


/*void* buscarYAsignarMarcoLibre(int pid, nodo_segmento nodoSegmento, nodo_paginas nodoPagina)
{
	int i;
	//Recorro la tabla de marcos
	for (i = 0; i<cantidadMarcos; i++)
	{
		//Uso el primer marco libre que encuentro
		if (tablaMarcos[i].libre == 1)
		{
			//Setteo los nuevos valores del marco, para que ahora aloje a la página en cuestión
			tablaMarcos[i].libre = 0;
			tablaMarcos[i].nro_pagina = nodoPagina->nro_pagina;
			tablaMarcos[i].nro_segmento = nodoSegmento->numeroSegmento;
			tablaMarcos[i].pid = nodoSegmento->pid;
			//Ahora, la página está presente en el número de marco, y el número de marco es i
			nodoPagina->presencia = i;
			//Devuelvo la dirección de memoria a la que apunta el marco, que es el bloque
			//de 256 bytes de tamaño.
			return (tablaMarcos[i].dirFisica);
		}
	}
	return NULL;
}*/
