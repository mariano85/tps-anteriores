/*
 * msp.c
 *
 *  Created on: 26/09/2014
 *      Author: utnso
 */
#include "msp.h"



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

	//char* path = malloc(13);
	//path = "configuracion";

	config = config_create("configuracion");

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

	//memcpy(configuracion.sust_pags, algoritmo, strlen(algoritmo));
	/*memcpy(&(configuracion.puerto), &puerto, sizeof(int));
	memcpy(&(configuracion.cantidad_memoria), &tamanio, sizeof(int));
	memcpy(&(configuracion.cantidad_swap), &swap, sizeof(int));*/

	configuracion.sust_pags = algoritmo;
	configuracion.puerto = puerto;
	configuracion.cantidad_memoria = tamanio;
	configuracion.cantidad_swap = swap;

	//config_destroy(config);

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
		log_error(logs, "ERROR: no se pudo crear la tabla de marcos.");
		abort();
	}

	void* buffer = malloc(TAMANIO_PAGINA);
	buffer = string_repeat('\0', TAMANIO_PAGINA);
	int i;
	for (i=0; i<cantidadMarcos; i++)
	{

		tablaMarcos[i].nro_marco = i;

		tablaMarcos[i].dirFisica = memoriaPrincipal + i*TAMANIO_PAGINA;

		tablaMarcos[i].orden = 0;

		//Cuando recién se crea la tabla, todos los marcos están libres.
		tablaMarcos[i].libre = 1;

		void* direccionDestino = tablaMarcos[i].dirFisica;

		memcpy(direccionDestino, buffer, TAMANIO_PAGINA);
	}

	free(buffer);
}

void listarMarcos()
{
	log_info(logs, "TABLA DE MARCOS");
	int i;
	for (i=0; i<cantidadMarcos; i++)
	{
		log_info(logs, "Numero marco: %d     PID: %d     Numero segmento: %d     Numero pagina: %d	   Libre: %d     Orden: %d", tablaMarcos[i].nro_marco, tablaMarcos[i].pid, tablaMarcos[i].nro_segmento, tablaMarcos[i].nro_pagina, tablaMarcos[i].libre, tablaMarcos[i].orden);
		printf("Numero marco: %d     PID: %d     Numero segmento: %d     Numero pagina: %d		", tablaMarcos[i].nro_marco, tablaMarcos[i].pid, tablaMarcos[i].nro_segmento, tablaMarcos[i].nro_pagina);
		printf("orden: %d    ", tablaMarcos[i].orden);
		printf("libre: %d     ", tablaMarcos[i].libre);
		printf("%.10s\n", (char*)tablaMarcos[i].dirFisica);
	}
}

void inicializarMSP()
{
	FILE *log = fopen("logMSP", "w");
	fflush(log);
	fclose(log);

	logs = log_create("logMSP", "MSP", 0, LOG_LEVEL_TRACE);

	configuracion = levantarArchivoDeConfiguracion();

	puts("algo");
	puts(configuracion.sust_pags);

	//Estas variables las uso para, cada vez que asigno un marco o swappeo una pagina, voy restando del
	//espacio total. Cuando alguna de estas dos variables llegue a 0, significa que no hay mas espacio.
	memoriaRestante = configuracion.cantidad_memoria;
	swapRestante = configuracion.cantidad_swap;

	printf("%d %d\n", memoriaRestante, swapRestante);

	memoriaPrincipal = malloc(configuracion.cantidad_memoria);
	if(memoriaPrincipal == NULL)
	{
		log_error(logs, "Error, no se pudo alocar espacio para la memoria principal.");
		abort();
	}

	crearTablaDeMarcos();

	listaSegmentos = list_create();

	ordenMarco = 0;

	log_trace(logs, "MSP inicio su ejecucion. Tamaño memoria: %d. Tamaño swap: %d", memoriaRestante, swapRestante);
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
	puts("CREACION DE SEGMENTO");
	printf("Nro seg: %d    PID: %d\n", nodoSegmento->numeroSegmento, nodoSegmento->pid);
	int i;
	for(i=0; i<cantidadDePaginas; i++)
	{
		nodo_paginas *nodoPagina;
		nodoPagina = list_get(listaPaginas, i);
		uint32_t direccion = generarDireccionLogica(nodoSegmento->numeroSegmento, nodoPagina->nro_pagina, 0);

		printf("		 Nro pag: %d      presencia: %d       direccion: %zu\n",  nodoPagina->nro_pagina, nodoPagina->presencia, direccion);

	}
	//Acá termina la impresion de prueba.

	uint32_t direccionBase = generarDireccionLogica(nodoSegmento->numeroSegmento, 0, 0);

	return direccionBase;

}

uint32_t crearSegmento(int pid, long tamanio)
{
	int cantidadTotalDePaginas;

	bool _pidCorresponde(nodo_segmento *p) {
			return (p->pid == pid);
		}
	t_list *listaDeSegmentosDeEstePid = list_filter(listaSegmentos, (void*)_pidCorresponde);
	int cantidadSegmentosDeEstePid = list_size(listaDeSegmentosDeEstePid);
	if (cantidadSegmentosDeEstePid == CANTIDAD_MAX_SEGMENTOS_POR_PID)
	{
		log_error(logs, "Error, ya no se pueden agregar más segmentos para PID %d. La cantidad máxima de segmentos por PID es de %d.", pid, CANTIDAD_MAX_SEGMENTOS_POR_PID);
		return EXIT_FAILURE;
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
		log_error(logs, "Error, no se puede crear el segmento para el PID %d porque excede el tamaño máximo de %d cantidad de páginas.", pid, CANTIDAD_MAX_PAGINAS_POR_SEGMENTO);
		return EXIT_FAILURE;
	}

	uint32_t direccionBase = agregarSegmentoALista(cantidadTotalDePaginas, pid, cantidadSegmentosDeEstePid);

	bool _ordenar(nodo_segmento *seg1, nodo_segmento *seg2)	{
		return (seg1->pid <= seg2->pid);
	}
	list_sort(listaSegmentos,(void*)_ordenar);

	int numeroSegmento, numeroPagina, offset;
	obtenerUbicacionLogica(direccionBase, &numeroSegmento, &numeroPagina, &offset);

	log_trace(logs, "Para el PID %d se creó el número de segmento %d de tamanio %d", pid, numeroSegmento, tamanio);

	return direccionBase;

}

int destruirSegmento(int pid, uint32_t base)
{
	int numeroSegmento, numeroPagina, offset;

	obtenerUbicacionLogica(base, &numeroSegmento, &numeroPagina, &offset);

	if (numeroPagina != 0 || offset != 0)
	{
		log_error(logs, "Error, la dirección proporcionada no es una dirección base.");
		return EXIT_FAILURE;
	}

	bool _pidYSegmentoCorresponde(nodo_segmento *p) {
		return (p->pid == pid && p->numeroSegmento == numeroSegmento);
	}

	nodo_segmento * nodo;

	if (!list_any_satisfy(listaSegmentos, (void*)_pidYSegmentoCorresponde))
	{
		log_error(logs, "Error, PID y/o segmento inválidos.");
		//printf("Error, pid y/o segmento invalidos\n");
		return EXIT_FAILURE;
	}
	else
	{
		nodo = list_remove_by_condition(listaSegmentos, (void*)_pidYSegmentoCorresponde);

		void _liberarMarcoOBorrarArchivoSwap(nodo_paginas *nodoPagina)
		{
			if (nodoPagina->presencia == -2)
			{
				char* nombreArchivo = string_new();

				nombreArchivo = generarNombreArchivo(nodo->pid, nodo->numeroSegmento, nodoPagina->nro_pagina);

				FILE *archivo = fopen(nombreArchivo, "w");
				remove(nombreArchivo);
				fclose(archivo);

				swapRestante = swapRestante + TAMANIO_PAGINA;

				log_trace(logs, "Se eliminó el archivo %s debido a la destrucción de la página %d del segmento %d del PID %d.", nombreArchivo, nodoPagina->nro_pagina, nodo->numeroSegmento, pid);
			}

			if ((nodoPagina->presencia >= 0))
			{
				int numeroMarco = nodoPagina->presencia;

				liberarMarco(numeroMarco, nodoPagina);

				log_trace(logs, "Se liberó el marco n° %d debido a la destrucción de la página %d del segmento %d del PID %d.", numeroMarco, nodoPagina->nro_pagina, nodo->numeroSegmento, pid);
			}

			free(nodoPagina);
		}

		list_iterate(nodo->listaPaginas, (void*)_liberarMarcoOBorrarArchivoSwap);

		free(nodo->listaPaginas);

		free(nodo);

		log_trace(logs, "Se destruyó el segmento %d del PID %d.", nodo->numeroSegmento, pid);
	}



	return EXIT_SUCCESS;
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
	log_info(logs, "TABLA DE SEGMENTOS");
	void _imprimirSegmento(nodo_segmento *nodoSegmento)
	{
		log_info(logs, "PID: %d       N° segmento: %d", nodoSegmento->pid, nodoSegmento->numeroSegmento);
	}

	list_iterate(listaSegmentos, (void*)_imprimirSegmento);

}

void tablaPaginas(int pid)
{
	t_list *listaFiltrada = filtrarListaSegmentosPorPid(listaSegmentos, pid);

	if(list_is_empty(listaFiltrada))
	{
		log_error(logs, "No se encontró el pid solicitado");
		//puts("Error, no se encontro el pid solicitado.");
		abort();
	}

	log_info(logs, "TABLA DE PÁGINAS DEL SEGMENTO %d.", pid);
	void imprimirDatosSegmento(nodo_segmento *nodoSegmento)
	{

		void imprimirDatosPaginas(nodo_paginas *nodoPagina)
		{
			printf("PID: %d\tNro segmento: %d\tNro Pagina: %d\tPresencia: %d\n", nodoSegmento->pid, nodoSegmento->numeroSegmento, nodoPagina->nro_pagina, nodoPagina->presencia);
			log_info(logs, "Nro segmento: %d\tPID: %d\tNro Pagina: %d\tPresencia: %d", nodoSegmento->numeroSegmento, nodoSegmento->pid, nodoPagina->nro_pagina, nodoPagina->presencia);
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
		log_error(logs, "Error, el PID ingresado no existe.");
		//puts("Error, el pid ingresado no existe.");
		return NULL;
	}

	int numeroSegmento, numeroPagina, offset;
	obtenerUbicacionLogica(direccionLogica, &numeroSegmento, &numeroPagina, &offset);
	if (offset > TAMANIO_PAGINA - 1)
	{
		//printf("numeroSegmento: %d   pid: %d   pag: %d   direccion: %d", numeroSegmento, pid, numeroPagina, (int)direccionLogica);
		log_error(logs, "Error, la dirección ingresada es inválida.");
		//puts("Error, la dirección ingresada es inválida.");
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
		//puts("Error, la dirección ingresada es inválida.");
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
		//puts("Error, la dirección ingresada es inválida.");
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
			//puts("Error, violacion de segmento.");
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
	void* buffermini; //Este es el buffer auxiliar que uso para copiar lo que
						//está en cada cachito de memoria. Despues lo muevo al buffer definitivo, es un una cuestion de orden de los cachos.
	int numeroSegmento, numeroPagina, offset;
	t_list* paginasQueNecesito = validarEscrituraOLectura(pid, direccionLogica, tamanio);

	if (paginasQueNecesito == NULL) return NULL;

	obtenerUbicacionLogica(direccionLogica, &numeroSegmento, &numeroPagina, &offset);

	void _traerAMemoriaPaginasSwappeadasParaLeer(nodo_paginas *nodo)
	{
		if (nodo->presencia == -2)
		{
			moverPaginaDeSwapAMemoria(pid, numeroSegmento, nodo);
		}
	}

	list_iterate(paginasQueNecesito, (void*)_traerAMemoriaPaginasSwappeadasParaLeer);

	nodo_paginas *nodoPagina = list_get(paginasQueNecesito, 0);
	void* direccionOrigen = offset + tablaMarcos[nodoPagina->presencia].dirFisica;

	buffer = malloc(tamanio);
	memset(buffer, 0, tamanio+1);

	//copio lo que queda para completar la primera pagina
	int yaCopie = TAMANIO_PAGINA - offset;
	memcpy(buffer, direccionOrigen, yaCopie);


	//copio las paginas del medio
	int i;
	for (i=1; i<((list_size(paginasQueNecesito)) - 1); i++)
	{

		nodoPagina = list_get(paginasQueNecesito, i);
		direccionOrigen = offset + tablaMarcos[nodoPagina->presencia].dirFisica;
		buffermini = malloc(TAMANIO_PAGINA);
		memset(buffermini, 0, TAMANIO_PAGINA);
		memcpy(buffermini, direccionOrigen, TAMANIO_PAGINA);
		memcpy(buffer + yaCopie, buffermini, TAMANIO_PAGINA);
		yaCopie = yaCopie + TAMANIO_PAGINA;
	}

	//copio lo que me queda del tamanio de la ultima pagina
	if ((list_size(paginasQueNecesito) > 1)){
		nodoPagina = list_get(paginasQueNecesito, i);
		direccionOrigen = offset + tablaMarcos[nodoPagina->presencia].dirFisica;
		buffermini = malloc(tamanio - yaCopie);
		memset(buffermini, 0, tamanio - yaCopie);
		memcpy(buffermini, direccionOrigen, tamanio - yaCopie);
		memcpy(buffer + yaCopie, buffermini, tamanio - yaCopie);
	}

	free(buffermini);

	return buffer;
}

void moverPaginaDeSwapAMemoria(int pid, int segmento, nodo_paginas* nodoPagina)
{
	char* nombreArchivo = string_new();

	int pagina = nodoPagina->nro_pagina;

	nombreArchivo = generarNombreArchivo(pid, segmento, pagina);

	FILE *archivo = fopen(nombreArchivo, "r");


	if (memoriaRestante < TAMANIO_PAGINA)
	{
		elegirVictimaSegunFIFO();
	}

	void* buffer = malloc(TAMANIO_PAGINA);

	fread(buffer, 1, TAMANIO_PAGINA, archivo);

	remove(nombreArchivo);

	fclose(archivo);

	swapRestante = swapRestante + TAMANIO_PAGINA;

	buscarYAsignarMarcoLibre(pid, segmento, nodoPagina);

	int numeroMarco = nodoPagina->presencia;

	escribirEnMarco(numeroMarco, TAMANIO_PAGINA, buffer, 0, 0);

	log_trace(logs, "Se movió a memoria principal la página %d del segmento %d del PID %d.", pagina, segmento, pid);

	free(buffer);
	free(nombreArchivo);
}

int escribirMemoria(int pid, uint32_t direccionLogica, void* bytesAEscribir, int tamanio)
{
	t_list* paginasQueNecesito = validarEscrituraOLectura(pid, direccionLogica, tamanio);

	if (paginasQueNecesito == NULL) return EXIT_FAILURE;

	int numeroSegmento, numeroPagina, offset;
	obtenerUbicacionLogica(direccionLogica, &numeroSegmento, &numeroPagina, &offset);

	int cantidadPaginasQueNecesito = list_size(paginasQueNecesito);

	int yaEscribi = 0;
	int i;
	int quedaParaCompletarPagina = TAMANIO_PAGINA - offset;
	int tamanioRestante = tamanio - quedaParaCompletarPagina;

	for (i=0; i<cantidadPaginasQueNecesito; i++)
	{

		nodo_paginas* nodoPagina = list_get(paginasQueNecesito, i);

		if (nodoPagina->presencia == -1)
		{
			if(memoriaRestante < TAMANIO_PAGINA)
			{
				log_trace(logs, "No hay espacio en la memoria principal.");
				if (swapRestante < TAMANIO_PAGINA)
				{
					log_error(logs, "No hay espacio disponible en memoria secundaria.");
					return EXIT_FAILURE;
				}
				elegirVictimaSegunFIFO();
			}

			buscarYAsignarMarcoLibre(pid, numeroSegmento, nodoPagina);
		}

		if (nodoPagina->presencia == -2)
		{
			moverPaginaDeSwapAMemoria(pid, numeroSegmento, nodoPagina);
			ordenMarco = ordenMarco - 1; //Esto lo hago para que no se saltee ningun orden. Cuando traigo la pagina a memoria,
										//hago un escribirEnMarco que aumenta el orden de escritura, y cuando escribo en nuevo buffer
										//tambien, por lo que el orden se aumenta dos veces.
		}

		if (i == 0)
		{

			if(tamanio <= quedaParaCompletarPagina)
			{
				escribirEnMarco (nodoPagina->presencia, tamanio, bytesAEscribir, offset, 0);
			}
			else
			{
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

	return EXIT_SUCCESS;

}

void escribirEnMarco(int numeroMarco, int tamanio, void* bytesAEscribir, int offset, int yaEscribi)
{
	void* direccionDestino = offset + tablaMarcos[numeroMarco].dirFisica;

	ordenMarco ++;

	tablaMarcos[numeroMarco].orden = ordenMarco;

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

			log_trace(logs, "Se asignó el marco %d a la página %d del segmento %d del PID %d.", i, nodoPagina->nro_pagina, numeroSegmento, pid);

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
		pthread_t hiloKernel;

		if(header_conexion_MSP == CPU_TO_MSP_HANDSHAKE)
		{
			puts("hola cpu");
			pthread_create(&hilo, NULL, atenderACPU, NULL);
		}

		if(header_conexion_MSP == KERNEL_TO_CPU_HANDSHAKE)
		{
			puts("hola kernel");
			pthread_create(&hiloKernel, NULL, atenderAKernel, NULL);
		}
	}
}

void* atenderAKernel(void* socket_kernel)
{
	log_info(logs, "Se creo el hilo para atender al kernel, espero peticiones del kernel:");
	bool running = true;

	while(running){
		t_contenido mensaje;
		memset(mensaje, 0, sizeof(t_contenido));
		t_header headerK = recibirMensaje((int)socket_kernel, mensaje, logs);

		if(headerK == ERR_CONEXION_CERRADA){
			log_error(logs, "el Hilo que atiende el KERNEL se desconecto, "
					"por lo que no se puede continuar");
			running = false;
		} else if(headerK == KERNEL_TO_MSP_ELIMINAR_SEGMENTOS){

			char** split = string_get_string_as_array(mensaje);
			int pid = atoi(split[0]);
			uint32_t base = atoi(split[1]);
			int exito = EXIT_FAILURE;

			log_info(logs,"ELIMINAR SEGMENTO :%d",pid);

			exito = destruirSegmento(pid, base);

			enviarMensaje((int)socket_kernel, KERNEL_TO_MSP_ELIMINAR_SEGMENTOS, string_from_format("%d", exito), logs);
		} else if(headerK == KERNEL_TO_MSP_MEM_REQ){

			char** split = string_get_string_as_array(mensaje);
			int pid = atoi(split[0]);
			int tamanio = atoi(split[1]);
			int exito = EXIT_FAILURE;

			log_info(logs,"ELIMINAR SEGMENTO :%d",pid);

			exito = crearSegmento(pid, tamanio);

			enviarMensaje((int)socket_kernel, MSP_TO_KERNEL_SEGMENTO_CREADO, string_from_format("%d", exito), logs);


		} else if(headerK == KERNEL_TO_MSP_ENVIAR_BYTES){
			char** split = string_get_string_as_array(mensaje);
			char *buffer = NULL;
			int pid = atoi(split[0]);
			int tamanio = atoi(split[1]);
			uint32_t direccion = atoi(split[2]);
			int exito = EXIT_FAILURE;

			enviarMensaje((int)socket_kernel, MSP_TO_KERNEL_ENVIO_BYTES, string_from_format("Ahora esperamos el buffer de escritura del proceso %d", pid), logs);

			buffer = calloc(tamanio, 1);

			recibirMensaje((int)socket_kernel, mensaje, logs);

			memcpy(buffer, mensaje, tamanio);

			exito = escribirMemoria(pid, direccion, buffer, tamanio);
			enviarMensaje((int)socket_kernel, MSP_TO_KERNEL_ENVIO_BYTES, string_from_format("%d", exito), logs);
		}

	}

	return EXIT_SUCCESS;

}

void* atenderACPU(void* socket_cpu)
{
	bool socketValidador = true;
	t_contenido mensaje_aux;

	crearSegmento(1234, 20);
	crearSegmento(1234, 40);
	log_info(logs,"Primer Escribir Memoria");
	escribirMemoria(1234, 1048576, "LOADA3456GETMABADDRBDSUBRDCMULRAB",33);

	while(socketValidador){

		t_contenido mensaje;
		memset(mensaje,0,sizeof(t_contenido));
		t_header header_recibido = recibirMensaje((int)socket_cpu, mensaje, logs);

		//Cargo lo que recibi --> el pir,dir_logica y tamanio

		char** array = string_get_string_as_array(mensaje);
		int pid = atoi(array[0]);
		uint32_t dir_logica = atoi(array[1]);
		int tamanio = atoi(array[2]);


		log_info(logs,"pid es %d,dir_logica es %d,tamanio es %d",pid,dir_logica,tamanio);

		void* buffer_instruccion = malloc(sizeof(int32_t));
		memset(buffer_instruccion,0,sizeof(int32_t));
		buffer_instruccion = solicitarMemoria(pid,dir_logica,tamanio);

		log_info(logs,"seguimiento de la dir_logica que es : %d",dir_logica);

		if(header_recibido == CPU_TO_MSP_SOLICITAR_BYTES){

					//Envio Instruccion
					t_contenido mensaje_instruccion;
					memset(mensaje_instruccion,0,sizeof(t_contenido));
					memcpy(mensaje_instruccion,buffer_instruccion,4);
					enviarMensaje((int)socket_cpu,MSP_TO_CPU_BYTES_ENVIADOS,mensaje_instruccion,logs);

					//Con este header --> Recibo lo que pido despues de pedir la instruccion --> puede ser REGISTRO -- DIRECCION - NUMERO
					recibirMensaje((int)socket_cpu,mensaje_aux,logs);
					char** array = string_get_string_as_array(mensaje_aux);

					int32_t program_counter = atoi(array[0]);
					int32_t auxiliar_cant_bytes = atoi(array[1]);

					log_info(logs,"el auxiliar vale %d y el program_counter %d",auxiliar_cant_bytes,program_counter);


					void* buffer_parametros = solicitarMemoria(pid,program_counter,auxiliar_cant_bytes);
					t_contenido mensaje_parametros;
					memset(mensaje_parametros,0,sizeof(t_contenido));
					memcpy(mensaje_parametros, buffer_parametros,auxiliar_cant_bytes);
					enviarMensaje((int)socket_cpu,MSP_TO_CPU_BYTES_ENVIADOS,mensaje_parametros,logs);

		}

	}//Fin while



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

	free(pidStr);
	free(numeroPaginaStr);
	free(numeroSegmentoStr);

	return nombreArchivo;
}

void elegirVictimaSegunFIFO()
{
	nodo_segmento *nodoSegmento;
	nodo_paginas *nodoPagina;
	t_list *listaPaginasDelSegmento;
	t_list *listaSegmentosDelPid;

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

	listaSegmentosDelPid = filtrarListaSegmentosPorPid(listaSegmentos, nodoMarco.pid);

	nodoSegmento = list_get(listaSegmentosDelPid, numeroSegmento);

	listaPaginasDelSegmento = nodoSegmento->listaPaginas;

	nodoPagina = list_get(listaPaginasDelSegmento, numeroPagina);

	crearArchivoDePaginacion(nodoSegmento->pid, nodoSegmento->numeroSegmento, nodoPagina);

	liberarMarco(nodoMarco.nro_marco, nodoPagina);

	log_trace(logs, "Se desalojó de memoria principal a la página %d del segmento %d del PID %d. El marco liberado es el n° %d.", nodoPagina->nro_pagina, nodoSegmento->numeroSegmento, nodoSegmento->pid, nodoMarco.nro_marco);
}

void liberarMarco(int numeroMarco, nodo_paginas *nodoPagina)
{
	tablaMarcos[numeroMarco].libre = 1;
	tablaMarcos[numeroMarco].nro_pagina = 0;
	tablaMarcos[numeroMarco].nro_segmento = 0;
	tablaMarcos[numeroMarco].orden = 0;
	tablaMarcos[numeroMarco].pid = 0;

	nodoPagina->presencia = -2;

	void* direccionDestino = tablaMarcos[numeroMarco].dirFisica;
	void* buffer = malloc(TAMANIO_PAGINA);
	buffer = string_repeat('\0', TAMANIO_PAGINA);

	memcpy(direccionDestino, buffer, TAMANIO_PAGINA);

	free(buffer);

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

	int numeroMarco = nodoPagina->presencia;

	void* direccionDestino = tablaMarcos[numeroMarco].dirFisica;

	fwrite(direccionDestino, 1, TAMANIO_PAGINA, archivoPaginacion);

	fclose(archivoPaginacion);

	log_trace(logs, "Se creó el archivo de swap %s para alojar a la página %d del segmento %d del PID %d.", nombreArchivo, nodoPagina->nro_pagina, numeroSegmento, pid);

	free(nombreArchivo);


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




