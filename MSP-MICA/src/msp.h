/*
 * msp.h
 *
 *  Created on: 26/09/2014
 *      Author: utnso
 */

#ifndef MSP_H_
#define MSP_H_


#include <stdbool.h>
#include <commons/log.h>
#include <commons/collections/list.h>




//Esta estructura me sirve para guardar todos los parametros de configuracion
typedef struct
{
	int puerto;
	long cantidad_memoria;
	int cantidad_swap;
	char* sust_pags;
} t_configuracion;


typedef struct
{
	int pid;
	int numeroSegmento;
	t_list* listaPaginas;
} nodo_segmento;

typedef struct
{
	long nro_marco;
	int pid;
	int nro_segmento;
	int nro_pagina;
} nodo_marcos;

typedef struct
{
	long nro_pagina;
	long presencia; //si vale -1, esta en swap, si no indica numero de marco
} nodo_paginas;



t_list* listaSegmentos;

t_list* listaMarcos;

t_configuracion configuracion;

t_log *logs;

char* ptoMP;





void tablaPaginas(int pid);

t_list* crearListaPaginas(int cantidadDePaginas);

void crearSegmento(int pid, long tamanio);

void agregarSegmentoALista(int cantidadDePaginas, int pid);

void tablaSegmentos();

t_configuracion levantarArchivoDeConfiguracion();

void inicializarMSP();

void listarMarcos();

void crearTablaDeMarcos();






#endif /* MSP_H_ */
