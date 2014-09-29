/*
 * programa.h
 *
 *  Created on: 19/04/2014
 *      Author: utnso
 */

#ifndef PROGRAMA_H_
#define PROGRAMA_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <commons/log.h>
#include <commons/temporal.h>
#include <commons/config.h>
#include <commons/comunicacion.h>

#include <unistd.h>

#define PROGRAMA_LOG_PATH "proceso_consola.log"
#define PROGRAMA_CONF_PATH "ESO.conf"

#define SALUDO "!!!Hola, soy un programa!!!"
#define LONG_MAX_LINEA  1024
#define LONG_IP 15+1
#define IP_SERVER_KEY "ip_kernel"
#define PORT_SERVER_KEY "puerto_kernel"

// variables globales del proceso
t_log *logger;
char *tiempo;
t_config * CONFIG;
t_log * LOGGER;
FILE * ARCHIVO_ANSISOP;

// firma de las funciones

void iniciarPrograma();
char* getLiteralFromFile(FILE* entrada);
char *getIpKernel();
int32_t getPuertoKernel();
void finalizarPrograma();

#endif /* PROGRAMA_H_ */
