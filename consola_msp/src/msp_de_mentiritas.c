#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "consola/commons/string.h"
#include "consola/commons/log.h"
#include "consola/consola_msp.h"


void tablaPaginas(int pid){ }

//t_list* crearListaPaginas(int cantidadDePaginas);

uint32_t crearSegmento(int pid, long tamanio){return 1234567890;}

//uint32_t agregarSegmentoALista(int cantidadDePaginas, int pid, int numeroSegmento);

void tablaSegmentos(){}





void listarMarcos(){}

//void crearTablaDeMarcos();

//uint32_t generarDireccionLogica(int numeroSegmento, int numeroPagina, int offset);

//void obtenerUbicacionLogica(uint32_t direccion, int *numeroSegmento, int *numeroPagina, int *offset);

void destruirSegmento(int pid, int base){}

//void escribirMemoria(int pid, uint32_t direccionLogica, void* bytesAEscribir, int tamanio);

//t_list* filtrarListaSegmentosPorPid(t_list* listaSegmentos, int pid);

//void* buscarYAsignarMarcoLibre(int pid, int numeroSegmento, nodo_paginas *nodoPagina);

//uint32_t obtenerUltimaDireccionSegmento(nodo_segmento* nodoSegmento);

//t_list* validarEscrituraOLectura(int pid, uint32_t direccionLogica, int tamanio);

//t_list* paginasQueVoyAUsar(nodo_segmento *nodoSegmento, int numeroPagina, int cantidadPaginas);

void escribirMemoria(int pid, int direccionLogica, void* bytesAEscribir, int tamanio){}
