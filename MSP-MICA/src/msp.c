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


	//Acá guardo la ruta del archivo de configuración
	char* path = "configuracion";

	//Abro el archivo de configuracion
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

	//Pido los valores de configuracion y los vuelco en una estructura
	algoritmo = config_get_string_value(config, "SUST_PAGS");
	puerto = config_get_int_value(config, "PUERTO");
	tamanio = config_get_int_value(config, "CANTIDAD_MEMORIA");
	swap = config_get_int_value(config, "CANTIDAD_SWAP");

	configuracion.sust_pags = algoritmo;
	configuracion.puerto = puerto;
	configuracion.cantidad_memoria = tamanio;
	configuracion.cantidad_swap = swap;

	//Devuelvo la estructura cuyos campos son los parametros de configuracion
	return configuracion;
}

void crearTablaDeMarcos()
{
	//Si las páginas tienen un tamaño fijo de 256 bytes, entonces la cantidad total de marcos que va a
	//haber en la MP del sistema es de tamaño memoria / 256, porque en un marco de memoria entra sólo
	//una página.
	cantidadMarcos = configuracion.cantidad_memoria / 256;

	//Aloco espacio de memoria para una tabla que va tener toda la información de los marcos de memoria.
	//calloc recibe dos parámetros por separado, la cantidad de espacios que necesito, y el tamaño de cada
	//uno de esos espacios, y además, settea cada espacio con el caracter nulo.
	tablaMarcos = calloc(cantidadMarcos, sizeof(t_marco));

	//Si por algún motivo calloc no pudo reservar la memoria, error.
	if (tablaMarcos == NULL)
	{
		puts("Error, no se pudo crear la tabla de marcos");
		log_trace(logs, "ERROR: no se pudo crear la tabla de marcos.");
		//Y si no hay tabla de marcos, no hay MSP, por lo cual chau proceso.
		abort();
	}

	//Recorro la tabla de marcos recién creada
	int i;
	for (i=0; i<cantidadMarcos; i++)
	{
		//Y setteo los datos de la estructura. Antes de este setteo, todos los campos valen el caracter nulo,
		//gracias a calloc. Pero igualmente necesito algunos datos para empezar a usar esta tabla.
		tablaMarcos[i].nro_marco = i;
		//Este es un puntero al bloque de memoria de 256 bytes al que apunta cada entrada de la tabla.
		//a.k.a "mallocs bebes". El gran bloque de memoria principal va a estar entonces formado por todos los bloquecitos
		//a los que apunta cada entrada de la tabla.
		tablaMarcos[i].dirFisica = NULL;
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
	//Abro el archivo de log
	logs = log_create("logMSP", "MSP", 0, LOG_LEVEL_TRACE);
	//Registro el inicio de ejecucion de la MSP
	log_trace(logs, "MSP inicio su ejecucion");

	//Obtengo los parametros de configuracion del archivo de configuracion.
	configuracion = levantarArchivoDeConfiguracion();

	//Estas variables las uso para, cada vez que asigno un marco o swappeo una pagina, voy restando del
	//espacio total. Cuando alguna de estas dos variables llegue a 0, significa que no hay mas espacio.
	memoriaRestante = configuracion.cantidad_memoria;
	swapRestante = configuracion.cantidad_swap;

	//Creo la tabla de marcos
	crearTablaDeMarcos();

	//Creo la lista se segmentos
	listaSegmentos = list_create();


	//CONECTAR CON KERNEL
	//ABRIR CONEXIONES CON CPU

	//Abro el archivo de log
	logs = log_create("logMSP", "MSP", 0, LOG_LEVEL_TRACE);
	//Registro el inicio de ejecucion de la MSP
	log_trace(logs, "MSP inicio su ejecucion. Tamaño memoria: %d", configuracion.cantidad_memoria, "Tamaño swap: %d", configuracion.cantidad_swap);

	//Reservo el gran bloque de memoria que va a actuar como MP
	ptoMP = malloc(configuracion.cantidad_memoria);
	if (ptoMP == NULL)
	{
		log_error(logs, "No es posible alocar memoria principal.");
		puts("Error, no es posible alocar memoria principal.");
		abort();
	}




	free(ptoMP);
}

void agregarSegmentoALista(int cantidadDePaginas, int pid, int cantidadSegmentosDeEstePid)
{
	//Creo una variable para alojar al nodo que voy a agregar a la lista de segmentos
	nodo_segmento *nodoSegmento;
	nodoSegmento = malloc(sizeof(nodo_segmento));

	//Creo una lista de paginas con la cantidad de paginas que le corresponden a este segmento
	t_list *listaPaginas;
	listaPaginas = crearListaPaginas(cantidadDePaginas);

	//Cargo en la estructura del nodo del segmento la información correspondiente
	nodoSegmento->numeroSegmento = cantidadSegmentosDeEstePid;
	nodoSegmento->pid = pid;
	nodoSegmento->listaPaginas = listaPaginas;

	//Agrego el segmento a la lista
	list_add(listaSegmentos, nodoSegmento);

	//Ordeno la lista de segmentos para que cuando pida la tabla de segmentos se imprima lindo



	printf("Nro seg: %d    PID: %d\n", nodoSegmento->numeroSegmento, nodoSegmento->pid);

	int i;
	for(i=0; i<cantidadDePaginas; i++)
	{
		nodo_paginas *nodoPagina;
		nodoPagina = list_get(listaPaginas, i);
		uint32_t direccion = generarDireccionLogica(nodoSegmento->numeroSegmento, nodoPagina->nro_pagina, 0);

		printf("		 Nro pag: %d      presencia: %d       direccion: %zu\n",  nodoPagina->nro_pagina, nodoPagina->presencia, direccion);

	}

}

void crearSegmento(int pid, long tamanio)
{
	int cantidadTotalDePaginas;

	//Valido si me queda memoria para agregar este segmento
	if (memoriaRestante < tamanio && swapRestante < tamanio)
	{
		printf("Error, no hay memoria ni espacio de swap suficiente.\n");
		log_trace(logs, "Error, no hay memoria ni espacio de swap suficiente.");
		abort();
	}

	//Me fijo que no haya 4096 segmentos para este pid
	bool _pidCorresponde(nodo_segmento *p) {
			return (p->pid == pid);
		}
	t_list *listaDeSegmentosDeEstePid = list_filter(listaSegmentos, (void*)_pidCorresponde);
	int cantidadSegmentosDeEstePid = list_size(listaDeSegmentosDeEstePid);
	if (cantidadSegmentosDeEstePid == CANTIDAD_MAX_SEGMENTOS_POR_PID)
	{
		printf("Error, ya hay 4096 segmentos para este PID, no se puede agregar nignuno más.");
		log_trace(logs, "Error, ya hay 4096 segmentos para este PID, no se puede agregar nignuno más.");
		return;
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

	//Me fijo que el tamaño que me pasaron por parámetro, no supere el tamaño máximo del segmento,
	//que es de 4096 paginas por segmento.
	if (cantidadTotalDePaginas > CANTIDAD_MAX_PAGINAS_POR_SEGMENTO)
	{
		log_error(logs, "Error, no se puede crear el segmento porque excede el tamaño máximo.");
		puts("Error, no se puede crear el segmento porque excede el tamaño máximo.");
		return;
	}

	//Agrego el segmento a la lista de segmentos.
	agregarSegmentoALista(cantidadTotalDePaginas, pid, cantidadSegmentosDeEstePid);

}

void destruirSegmento(int pid, uint32_t base)
{
	int numeroSegmento, numeroPagina, offset;

	//Llamo a esta función que hace una cuenta loca y, en base una una *base* de segmento, me devuelve el numero se segmento al que
	//corresponde esa base, y el número de página y el offset también, pero en el caso de destruir segmento
	//no los necesito porque ambos son 0, debido a que se trata de la base del segmento.
	obtenerUbicacionLogica(base, &numeroSegmento, &numeroPagina, &offset);

	//Esta es la función con la que opero la lista, me fijo que exista el PID que me mandan por parámetro
	//y también que ese pid tenga un segmento con el número correspondiente a la base.
	bool _pidYSegmentoCorresponde(nodo_segmento *p) {
		return (p->pid == pid && p->numeroSegmento == numeroSegmento);
	}

	nodo_segmento * nodo;


	//Si ninguna lista satisface la condición anterior, error.
	if (!list_any_satisfy(listaSegmentos, (void*)_pidYSegmentoCorresponde))
	{
		log_error(logs, "Error, PID y/o segmento inválidos.");
		printf("Error, pid invalido\n");
		return;
	}
	else
	{
		//Borro de la lista el nodo que corresponde al segmento solicitado
		nodo = list_remove_by_condition(listaSegmentos, (void*)_pidYSegmentoCorresponde);
		//Dentro del nodo segmento, hay una lista te páginas, que como es memoria dinámica, también necesita ser liberada.
		free(nodo->listaPaginas);
		//Ahora sí, libero el nodo. Chau memory leaks! (creo)
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

void escribirMemoria(int pid, uint32_t direccionLogica, void* bytesAEscribir, int tamanio)
{

}
