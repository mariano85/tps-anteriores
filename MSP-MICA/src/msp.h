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
#include <commons/config.h>
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
	int libre;	//si vale 1, el marco está ocupado, si vale 0 está libre
	int orden;
} t_marco;

typedef struct
{
	int nro_pagina;
	int presencia;	//si vale -1, no se le asignó ningun marco, si vale -2 está en swap, si no indica numero de marco
} nodo_paginas;





//VARIABLES globales

int ordenMarco;

t_list* listaSegmentos;

t_configuracion configuracion;

t_log *logs;

void* memoriaPrincipal;

t_marco *tablaMarcos;

int memoriaRestante;

int swapRestante;

int cantidadMarcos;





//FUNCIONES

void tablaPaginas(int pid);

t_list* crearListaPaginas(int cantidadDePaginas);

uint32_t crearSegmento(int pid, long tamanio);

uint32_t agregarSegmentoALista(int cantidadDePaginas, int pid, int numeroSegmento);

void tablaSegmentos();

t_configuracion levantarArchivoDeConfiguracion();

void inicializarMSP();

void listarMarcos();

void crearTablaDeMarcos();

uint32_t generarDireccionLogica(int numeroSegmento, int numeroPagina, int offset);

void obtenerUbicacionLogica(uint32_t direccion, int *numeroSegmento, int *numeroPagina, int *offset);

void destruirSegmento(int pid, uint32_t base);

t_list* filtrarListaSegmentosPorPid(t_list* listaSegmentos, int pid);

void* buscarYAsignarMarcoLibre(int pid, int numeroSegmento, nodo_paginas *nodoPagina);

t_list* validarEscrituraOLectura(int pid, uint32_t direccionLogica, int tamanio);

t_list* paginasQueVoyAUsar(nodo_segmento *nodoSegmento, int numeroPagina, int cantidadPaginas);

void escribirMemoria(int pid, uint32_t direccionLogica, void* bytesAEscribir, int tamanio);

void escribirEnMarco(int numeroMarco, int tamanio, void* bytesAEscribir, int offset, int yaEscribi);

void *solicitarMemoria(int pid, uint32_t direccionLogica, int tamanio);

void conexionConKernelYCPU();

void* atenderACPU(void *socket_cpu);

void* atenderAKernel(void* socket_msp);

int crearArchivoDePaginacion(int pid, int numeroSegmento, nodo_paginas *nodoPagina);

char* generarNombreArchivo(int pid, int numeroSegmento, int numeroPagina);

void elegirVictimaSegunFIFO();

void liberarMarco(int numeroMarco, nodo_paginas *nodoPagina);

uint32_t aumentarProgramCounter(uint32_t programCounterAnterior, int bytesASumar);




#endif /* MSP_H_ */
