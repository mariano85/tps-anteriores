/*
 * tad_nivel.c
 *
 *  Created on: 08/10/2013
 *      Author: elyzabeth
 */

#include "tad_nivel.h"

t_nivel * crearNivel ( char *nombre, int32_t fdNivel) {
	t_nivel *nuevoNivel;
	nuevoNivel = (t_nivel*)malloc(sizeof(t_nivel));

	strcpy(nuevoNivel->nombre, nombre);
	nuevoNivel->fdSocket = fdNivel;

	return nuevoNivel;
}

void initNivel(t_nivel *nivel) {
	memset(nivel, '\0', sizeof(t_nivel));
}

void destruirNivel (t_nivel *nivel) {
	free(nivel);
}

void imprimirNivel (t_nivel *n, t_log *LOGGER) {
	log_info(LOGGER, "\r -- Nivel -> nombre: '%s' - algoritmo: '%s' - quantum: %d - retardo: %d - socket: '%d' - ", n->nombre, n->algoritmo, n->quantum, n->retardo, n->fdSocket);
}
