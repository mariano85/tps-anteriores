/*
 * umv_prueba.h
 *
 *  Created on: 02/05/2014
 *      Author: utnso
 */

#ifndef UMV_PRUEBA_H_
#define UMV_PRUEBA_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/temporal.h>
#include <commons/config.h>
#include <commons/collections/list.h>

#define UMV_LOG_PATH "umv.log"
#define UMV_CONF_PATH "umv.conf"

#define ERROR		    0
#define EXITO		    1
#define WARNING         2

#define CANT_TIPO_SEGMENTOS 4

const static char *TIPO_SEGMENTO_NOMBRES[CANT_TIPO_SEGMENTOS] = {
		"SEGMENTO_CODIGO_LITERAL"
		, "SEGMENTO_INDICE_DE_ETIQUETAS"
		, "SEGMENTO_INDICE_DE_FUNCIONES"
		, "SEGMENTO_STACK"};

const static char *BOOL_NOMBRE[2] = {"_FALSE", "_TRUE"};

typedef enum tipo_segmento {
	SEGMENTO_CODIGO_LITERAL,
	SEGMENTO_INDICE_DE_ETIQUETAS,
	SEGMENTO_INDICE_DE_FUNCIONES,
	SEGMENTO_STACK
} TIPO_SEGMENTO;

typedef enum tipo_ubi {
	WORST_FIT,
	FIRST_FIT
} TIPO_ALGORITMO_UBICACION;

// esto es una estructura que pertenece al plp
typedef struct _t_segmento_plp {
	int32_t pid;
	uint32_t inicio[CANT_TIPO_SEGMENTOS];
	uint32_t tamanio[CANT_TIPO_SEGMENTOS];
	int32_t direccionFisica[CANT_TIPO_SEGMENTOS]; // si tiene -1 es porque no está alojado en la UMV
} t_segmento_plp;

typedef struct _t_segmento_umv {
	int32_t pid;
	TIPO_SEGMENTO tipo;
	uint32_t direccionFisica;
	uint32_t tamanio;
	bool disponible; // este seria el borrado logico, cuando disponible == false, seria un segmento eliminado
} t_segmento_umv;

// variables globales del proceso
uint32_t ALGORITMO_UBICACION;
t_log *LOGGER;
char *UMV_PTR;
size_t UMV_SIZE;
t_list *listaSegmentosUMV;

// definiciones de las funciones que voy a usar

t_segmento_plp* crearSegmentoPLP(int32_t pid
		, int32_t tam1
		, int32_t tam2
		, int32_t tam3
		, int32_t tam4);

int32_t getTamanioSegmentoFromBase(uint32_t base);
void compactacion();
void crearSegmentos(t_segmento_plp *msg_plp);
void inhabilitarSegmentos(int32_t pid);
void destruirSegmentos(int32_t pid);
void iniciarUMV();
void finalizarUMV();
bool compararSegmentos(t_segmento_umv* segmento1, t_segmento_umv* segmento2);
t_segmento_umv *getUltimoSegmento();
int32_t obtenerDireccionLibre();
int32_t obtenerMemoriaDisponible();
void compactacion();

#endif /* UMV_PRUEBA_H_ */
