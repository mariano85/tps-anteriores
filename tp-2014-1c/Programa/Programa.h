/*
 * programa.h
 *
 *  Created on: 24/05/2013
 *      Author: utnso
 */

#ifndef PROGRAMA_H_
#define PROGRAMA_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// una negrada para usar el makefile y que no le moleste la compilacion a Jani
#ifdef COMPILAR_DESDE_CONSOLA

#include <Commons/commons/sockets.h>
#include <Commons/commons/string.h>
#include <Commons/commons/log.h>
#include <Commons/commons/config.h>
#include <Commons/commons/process.h>
#else

#include "commons/sockets.h"
#include "commons/string.h"
#include "commons/log.h"
#include "commons/config.h"
#include "commons/process.h"

#endif

/* FUNCIONES ! */

void cargarConfiguracion(char* pathArchiConf);
void hacerHandshakeConKernel(int32_t socket);
void imprimir_en_pantalla(char* mensaje);
void imprimir_en_impresora(char * mensaje);
char* getCodigoAnsisop(char* path);

#endif /* PROGRAMA_H_ */
