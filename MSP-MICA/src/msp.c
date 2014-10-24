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
#include <string.h>
#include <commons/sockets.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>
#include <commons/temporal.h>


#define CANTIDAD_MAX_SEGMENTOS_POR_PID 3
#define CANTIDAD_MAX_PAGINAS_POR_SEGMENTO 5
#define TAMANIO_PAGINA 10


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
	if (configuracion.cantidad_memoria % TAMANIO_PAGINA == 0)
	{
		cantidadMarcos = configuracion.cantidad_memoria / TAMANIO_PAGINA;
	}
	else
	{
		//Si la división no es exacta, necesito un marco más para alojar lo que queda de la memoria.
		cantidadMarcos = configuracion.cantidad_memoria / TAMANIO_PAGINA + 1;
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

		tablaMarcos[i].dirFisica = memoriaPrincipal + i*TAMANIO_PAGINA;

		tablaMarcos[i].orden = 0;

		//Cuando recién se crea la tabla, todos los marcos están libres.
		tablaMarcos[i].libre = 1;

		void* direccionDestino = tablaMarcos[i].dirFisica;
		void* buffer = malloc(TAMANIO_PAGINA);
		buffer = string_repeat('\0', TAMANIO_PAGINA);
		memcpy(direccionDestino, buffer, TAMANIO_PAGINA);
	}
}

void listarMarcos()
{
	int i;
	for (i=0; i<cantidadMarcos; i++)
	{
		printf("Numero marco: %d     PID: %d     Numero segmento: %d     Numero pagina: %d		", tablaMarcos[i].nro_marco, tablaMarcos[i].pid, tablaMarcos[i].nro_segmento, tablaMarcos[i].nro_pagina);
		printf("orden: %d    ", tablaMarcos[i].orden);
		printf("libre: %d     ", tablaMarcos[i].libre);
		printf("%.10s\n", (char*)tablaMarcos[i].dirFisica);
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

	ordenMarco = 0;




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

	//Si el resto de la división entre el tamaño y TAMANIO_PAGINA (que es el tamaño máximo de la pagina) da 0...
	if (tamanio % TAMANIO_PAGINA == 0)
	{
		//...entonces la cantiadad de páginas es el resultado de la división, si no...
		cantidadTotalDePaginas = tamanio / TAMANIO_PAGINA;
	}
	else
	{
		//...la cantidad total de páginas es el resultado de la división más 1, para agregar en una página lo que
		//queda del tamaño, por supuesto, esta última página no estará completa.
		cantidadTotalDePaginas = tamanio / TAMANIO_PAGINA + 1;
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

	if(list_is_empty(listaFiltrada))
	{
		log_trace(logs, "No se encontró el pid solicitado");
		puts("Error, no se encontro el pid solicitado.");
		abort();
	}

	void imprimirDatosSegmento(nodo_segmento *nodoSegmento)
	{

		void imprimirDatosPaginas(nodo_paginas *nodoPagina)
		{
			printf("Nro segmento: %d\tPID: %d\tNro Pagina: %d\tPresencia: %d\n", nodoSegmento->numeroSegmento, nodoSegmento->pid, nodoPagina->nro_pagina, nodoPagina->presencia);
		}

		list_iterate(nodoSegmento->listaPaginas, (void*)imprimirDatosPaginas);
	}

	list_iterate(listaFiltrada, (void*)imprimirDatosSegmento);

}

uint32_t generarDireccionLogica(int numeroSegmento, int numeroPagina, int offset)
{
	uint32_t direccion = numeroSegmento;
	direccion = direccion << 12;
	direccion = direccion | numeroPagina;
	direccion = direccion << 8;
	direccion = direccion | offset;

	return direccion;
}

void obtenerUbicacionLogica(uint32_t direccion, int *numeroSegmento, int *numeroPagina, int *offset)
{
	*offset = direccion & 0xFF;
	*numeroPagina = (direccion >> 8) & 0xFFF;
	*numeroSegmento = (direccion >> 20) & 0xFFF;
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

t_list* validarEscrituraOLectura(int pid, uint32_t direccionLogica, int tamanio)
{

	t_list* listaFiltradaPorPid = filtrarListaSegmentosPorPid(listaSegmentos, pid);

	if (list_is_empty(listaFiltradaPorPid))
	{
		log_error(logs, "Error, el pid ingresado no existe.");
		puts("Error, el pid ingresado no existe.");
		return NULL;
	}

	int numeroSegmento, numeroPagina, offset;
	obtenerUbicacionLogica(direccionLogica, &numeroSegmento, &numeroPagina, &offset);
	if (offset > TAMANIO_PAGINA - 1)
	{
		printf("numeroSegmento: %d   pid: %d   pag: %d   direccion: %d", numeroSegmento, pid, numeroPagina, (int)direccionLogica);
		log_error(logs, "Error, la dirección ingresada es inválida.");
		puts("Error, la dirección ingresada es inválida.");
		return NULL;
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
		return NULL;
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
		return NULL;
	}

	//Tengo que transformar el tamanio en paginas, para ver si el segmneto tiene esa cantidad de paginas
	t_list* paginasQueNecesito;
	int cantidadPaginasQueOcupaTamanio;
	int faltaParaCompletarPagina = TAMANIO_PAGINA - offset;
	int quedaDelTamanio = tamanio - faltaParaCompletarPagina;
	if (quedaDelTamanio < 0)
	{
		cantidadPaginasQueOcupaTamanio = 1;
		paginasQueNecesito = paginasQueVoyAUsar(nodoSegmento, nodoPagina->nro_pagina, cantidadPaginasQueOcupaTamanio);
	}
	else
	{
		int cantidadPaginasEnterasQueQuedan = quedaDelTamanio / TAMANIO_PAGINA;
		//Me fijo si necesito una pagina para alojar el remanente
		if (quedaDelTamanio % TAMANIO_PAGINA != 0)
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
			return NULL;
		}

		paginasQueNecesito = paginasQueVoyAUsar(nodoSegmento, nodoPagina->nro_pagina, cantidadPaginasQueOcupaTamanio+1);
	}


	/*void _imprimirNumeroPagina(nodo_paginas *nodoPagina)
	{
		printf("voy a usar pagina: %d\n", nodoPagina->nro_pagina);
	}

	list_iterate(paginasQueNecesito, (void*)_imprimirNumeroPagina);*/

	return paginasQueNecesito;

}


void* solicitarMemoria(int pid, uint32_t direccionLogica, int tamanio)
{
	void* buffer;
	int numeroSegmento, numeroPagina, offset;
	t_list* paginasQueNecesito = validarEscrituraOLectura(pid, direccionLogica, tamanio);

	obtenerUbicacionLogica(direccionLogica, &numeroSegmento, &numeroPagina, &offset);

	nodo_paginas *nodoPagina = list_get(paginasQueNecesito, 0);

	void* direccionOrigen = offset + tablaMarcos[nodoPagina->presencia].dirFisica;


	buffer = malloc(tamanio);
	memcpy(buffer, direccionOrigen, tamanio);

	puts("BUFFER");
	puts(buffer);
	return buffer;

}

void escribirMemoria(int pid, uint32_t direccionLogica, void* bytesAEscribir, int tamanio)
{
	t_list* paginasQueNecesito = validarEscrituraOLectura(pid, direccionLogica, tamanio);

	if (paginasQueNecesito == NULL) return;

	int numeroSegmento, numeroPagina, offset;
	obtenerUbicacionLogica(direccionLogica, &numeroSegmento, &numeroPagina, &offset);

	int cantidadPaginasQueNecesito = list_size(paginasQueNecesito);
	int tamanioRestante = tamanio;
	int yaEscribi = 0;
	int i;

	for (i=0; i<cantidadPaginasQueNecesito; i++)
	{

		nodo_paginas* nodoPagina = list_get(paginasQueNecesito, i);

		if (nodoPagina->presencia == -1)
		{
			if (tamanio > memoriaRestante)
			{
					log_error(logs, "Error, no hay espacio suficiente en la memoria.");
					puts("Error, no hay espacio suficiente en la memoria.");
					return;


			}

			else
			{
				buscarYAsignarMarcoLibre(pid, numeroSegmento, nodoPagina);
			}
		}

		if (i == 0)
		{
			int quedaParaCompletarPagina = TAMANIO_PAGINA - offset;

			if(tamanio <= quedaParaCompletarPagina)
			{
				escribirEnMarco (nodoPagina->presencia, tamanio, bytesAEscribir, offset, 0);
			}
			else
			{
				tamanioRestante = tamanioRestante - quedaParaCompletarPagina;

				yaEscribi = yaEscribi + quedaParaCompletarPagina;

				escribirEnMarco (nodoPagina->presencia, quedaParaCompletarPagina, bytesAEscribir, offset, 0);
			}

		}

		if ((tamanioRestante / TAMANIO_PAGINA == 0) && (i!=0))
		{
			escribirEnMarco (nodoPagina->presencia, tamanioRestante, bytesAEscribir, 0, yaEscribi);
			tamanioRestante = tamanioRestante - (tamanioRestante % TAMANIO_PAGINA);
			yaEscribi = yaEscribi + (tamanioRestante % TAMANIO_PAGINA);

		}
		else if ((tamanioRestante / TAMANIO_PAGINA > 0) && (i!=0))
		{
			escribirEnMarco (nodoPagina->presencia, TAMANIO_PAGINA, bytesAEscribir, 0, yaEscribi);
			tamanioRestante = tamanioRestante - TAMANIO_PAGINA;
			yaEscribi = yaEscribi + TAMANIO_PAGINA;


		}

	}



}

void escribirEnMarco(int numeroMarco, int tamanio, void* bytesAEscribir, int offset, int yaEscribi)
{
	void* direccionDestino = offset + tablaMarcos[numeroMarco].dirFisica;

	tablaMarcos[numeroMarco].orden = ordenMarco;
	ordenMarco ++;

	memcpy(direccionDestino, bytesAEscribir + yaEscribi, tamanio);
}

t_list* paginasQueVoyAUsar(nodo_segmento *nodoSegmento, int numeroPagina, int cantidadPaginas)
{
	t_list* paginasQueNecesito;

	paginasQueNecesito = list_create();

	int i, ultimaPagina;

	ultimaPagina = numeroPagina + cantidadPaginas;

	for (i=numeroPagina; i<ultimaPagina; i++)
	{
		nodo_paginas* nodoPagina;

		nodoPagina = list_get(nodoSegmento->listaPaginas, i);

		list_add(paginasQueNecesito, nodoPagina);

	}

	return paginasQueNecesito;
}

void* buscarYAsignarMarcoLibre(int pid, int numeroSegmento, nodo_paginas *nodoPagina)
{
	int i;
	for (i = 0; i<cantidadMarcos; i++)
	{
		//Uso el primer marco libre que encuentro
		if (tablaMarcos[i].libre == 1)
		{
			memoriaRestante = memoriaRestante - TAMANIO_PAGINA;

			tablaMarcos[i].libre = 0;
			tablaMarcos[i].nro_pagina = nodoPagina->nro_pagina;
			tablaMarcos[i].nro_segmento = numeroSegmento;
			tablaMarcos[i].pid = pid;
			nodoPagina->presencia = i;
			return (tablaMarcos[i].dirFisica);
		}
	}

	return NULL;
}

void conexionConKernelYCPU()
{

	struct sockaddr_in my_addr, their_addr;

	int socketFD, newFD;

	socketFD = crearSocket();

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(configuracion.puerto);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), '\0', sizeof(struct sockaddr_in));

	bindearSocket(socketFD, my_addr);

	escucharEn(socketFD);

	while(1)
	{
		socklen_t sin_size;

		sin_size = sizeof(struct sockaddr_in);


		log_trace(logs, "A la espera de nuevas conexiones");

		if((newFD = accept(socketFD,(struct sockaddr *)&their_addr, &sin_size)) == -1)
		{
			perror("accept");
			continue;
		}

		log_trace(logs, "Recibí conexion de %s", inet_ntoa(their_addr.sin_addr));

		t_contenido mensajeParaRecibirConexionCpu;
		memset (mensajeParaRecibirConexionCpu, 0, sizeof(t_contenido));

		t_header header_conexion_MSP = recibirMensaje(newFD, mensajeParaRecibirConexionCpu, logs);

		pthread_t hilo;

		if(header_conexion_MSP == CPU_TO_MSP_HANDSHAKE)
		{
			pthread_create(&hilo, NULL, atenderACPU, NULL);


		}

	}







}

void *atenderACPU()
{
	printf("Hola\n");

	return EXIT_SUCCESS;
}

char* generarNombreArchivo(int pid, int numeroSegmento, int numeroPagina)
{
	char* nombreArchivo = string_new();
	char* pidStr = string_new();
	char* numeroPaginaStr = string_new();
	char* numeroSegmentoStr = string_new();

	pidStr = string_itoa(pid);
	numeroPaginaStr = string_itoa(numeroPagina);
	numeroSegmentoStr = string_itoa(numeroSegmento);

	nombreArchivo = string_from_format("%s_%s_%s", pidStr, numeroSegmentoStr, numeroPaginaStr);

	return nombreArchivo;
}

void elegirVictimaSegunFIFO()
{
	nodo_segmento *nodoSegmento;
	nodo_paginas *nodoPagina;
	t_list *listaPaginas;

	swapRestante = swapRestante - TAMANIO_PAGINA;

	t_marco nodoMarco = tablaMarcos[0];
	int i;
	for (i=1; i<cantidadMarcos; i++)
	{
		if((tablaMarcos[i].orden < nodoMarco.orden) && (tablaMarcos[i].pid != 0))
		{
			nodoMarco = tablaMarcos[i];
		}
	}

	int numeroSegmento = nodoMarco.nro_segmento;
	int numeroPagina = nodoMarco.nro_pagina;

	nodoSegmento = list_get(listaSegmentos, numeroSegmento);

	listaPaginas = nodoSegmento->listaPaginas;

	nodoPagina = list_get(listaPaginas, numeroPagina);

	crearArchivoDePaginacion(nodoSegmento->pid, nodoSegmento->numeroSegmento, nodoPagina);

	liberarMarco(nodoMarco.nro_marco, nodoPagina);
}

void liberarMarco(int numeroMarco, nodo_paginas *nodoPagina)
{
	tablaMarcos[numeroMarco].libre = 1;
	tablaMarcos[numeroMarco].nro_pagina = 0;
	tablaMarcos[numeroMarco].nro_segmento = 0;
	tablaMarcos[numeroMarco].orden = 0;
	tablaMarcos[numeroMarco].pid = 0;

	nodoPagina->presencia = -1;

	void* direccionDestino = tablaMarcos[numeroMarco].dirFisica;
	void* buffer = malloc(TAMANIO_PAGINA);
	buffer = string_repeat('\0', TAMANIO_PAGINA);

	memcpy(direccionDestino, buffer, TAMANIO_PAGINA);

	memoriaRestante = memoriaRestante + TAMANIO_PAGINA;
}



int crearArchivoDePaginacion(int pid, int numeroSegmento, nodo_paginas *nodoPagina)
{
	char* nombreArchivo = string_new();
	nombreArchivo = generarNombreArchivo(pid, numeroSegmento, nodoPagina->nro_pagina);

	FILE *archivoPaginacion = fopen(nombreArchivo, "w");

	if (archivoPaginacion == NULL)
	{
		log_error(logs, "No pudo abrirse el archivo de paginacion.");
		return EXIT_FAILURE;
	}

	char* buffer = malloc(TAMANIO_PAGINA);

	int numeroMarco = nodoPagina->presencia;

	char* direccionDestino = tablaMarcos[numeroMarco].dirFisica;

	memcpy(buffer, direccionDestino, TAMANIO_PAGINA);

	fprintf(archivoPaginacion, "%s", buffer);

	fclose(archivoPaginacion);

	return EXIT_SUCCESS;

}

uint32_t aumentarProgramCounter(uint32_t programCounterAnterior, int bytesASumar)
{
	uint32_t nuevoProgramCounter;
	int numeroSegmento, numeroPagina, offset;

	obtenerUbicacionLogica(programCounterAnterior, &numeroSegmento, &numeroPagina, &offset);

	if (offset + bytesASumar > TAMANIO_PAGINA)
	{
		int faltaParaCompletarPagina = TAMANIO_PAGINA - offset;
		int quedaParaSumar = bytesASumar - faltaParaCompletarPagina;
		int paginaFinal, offsetPaginaFinal;

		paginaFinal = numeroPagina + quedaParaSumar / TAMANIO_PAGINA + 1;
		offsetPaginaFinal = quedaParaSumar % TAMANIO_PAGINA;

		nuevoProgramCounter = generarDireccionLogica(numeroSegmento, paginaFinal, offsetPaginaFinal);


	}

	else
	{
		nuevoProgramCounter = generarDireccionLogica(numeroSegmento, numeroPagina, offset + bytesASumar);
	}


	return nuevoProgramCounter;
}




