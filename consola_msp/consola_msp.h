/*
 * consola_msp.h
 *
 *  Created on: 10/09/2014
 *      Author: alphard
 */

#ifndef CONSOLA_MSP_H_
#define CONSOLA_MSP_H_

typedef enum Comandos {
	CREAR_SEGMENTO,
	DESTRUIR_SEGMENTO,
	ESCRIBIR_MEMORIA,
	LEER_MEMORIA,
	TABLA_SEGMENTOS,
	TABLA_PAGINAS,
	LISTAR_MARCOS,
	HELP,
	ERROR,
	EXIT,
}t_comando;

t_comando buscar_comando(char* texto);
int texto_a_numero(char* texto);


#endif /* CONSOLA_MSP_H_ */
