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
	void* dirFisica;
	int libre; //si vale 0, el marco está ocupado, si vale 1 está libre
} t_marco;

typedef struct
{
	int nro_pagina;
	int presencia;	//si vale -1, no se le asignó ningun marco o está en swap, si no indica numero de marco
} nodo_paginas;





//VARIABLES LOCALES

t_list* listaSegmentos;

t_list* listaMarcos;

t_configuracion configuracion;

t_log *logs;

void* ptoMP;

t_marco *tablaMarcos;

int memoriaRestante;

int swapRestante;

int cantidadMarcos;





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

void obtenerUbicacionLogica(uint32_t direccion, int *numeroSegmento, int *numeroPagina, int *offset);

void destruirSegmento(int pid, uint32_t base);

void escribirMemoria(int pid, uint32_t direccionLogica, void* bytesAEscribir, int tamanio);




#endif /* MSP_H_ */
