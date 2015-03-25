/*
 * consola.h
 *
 *  Created on: 06/11/2014
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/sockets.h>
#include <commons/process.h>
#include <unistd.h>

#define PROGRAMA_LOG_PATH "consola.log"
#define PROGRAMA_CONF_PATH "BESO.conf"

#define LONG_MAX_LINEA  1024
#define LONG_IP 15+1
#define IP_SERVER_KEY "IP_KERNEL"
#define PORT_SERVER_KEY "PUERTO_KERNEL"

// variables globales del proceso
int32_t socketKernel;
t_config * CONFIG;
t_log * LOGGER;

void mostrar_registros(char* mensaje);
void salida_estandar(char* mensaje);
void entrada_estandar(char* mensaje);
void loadConfig();
char *getCodigoBESO(size_t *tamanioArchivo, FILE* entrada);
char *getIpKernel();
int32_t getPuertoKernel();
void hacerHandshakeConKernel(int32_t socket);
bool enviarCodigoBESO(char* nombreBESO, char* codigoBESO, size_t tamanio);

#endif /* CONSOLA_H_ */
