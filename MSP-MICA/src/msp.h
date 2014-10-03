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
#include <stdlib.h>
#include <stdint.h>



#define CANTIDAD_MAX_PAGINAS_TOTAL 100


//ESTRUCTURAS

//Esta estructura me sirve para guardar todos los parametros de configuracion
typedef struct
{
	int puerto;
	int cantidad_memoria;
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
	int nro_marco;
	int pid;
	int nro_segmento;
	int nro_pagina;
} t_marco;

typedef struct
{
	uint32_t nro_pagina;
	uint32_t presencia;	//si vale -1, esta en swap, si no indica numero de marco
	void* dirFisica;
} nodo_paginas;





//VARIABLES LOCALES

t_list* listaSegmentos;

t_list* listaMarcos;

t_configuracion configuracion;

t_log *logs;

void* ptoMP;

t_marco *tablaMarcos;





//FUNCIONES

void tablaPaginas(int pid);

t_list* crearListaPaginas(int cantidadDePaginas);

void crearSegmento(int pid, long tamanio);

void agregarSegmentoALista(int cantidadDePaginas, int pid, int numeroSegmento);

void tablaSegmentos();

t_configuracion levantarArchivoDeConfiguracion();

void inicializarMSP();

void listarMarcos();

void crearTablaDeMarcos();

uint32_t generarDireccionLogica(int numeroSegmento, int numeroPagina, int offset);





#endif /* MSP_H_ */
