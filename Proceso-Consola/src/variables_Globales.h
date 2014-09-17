/*
 * variaables_Globales.h
 *
 *  Created on: 17/09/2014
 *      Author: utnso
 */

#ifndef VARIAABLES_GLOBALES_H_
#define VARIAABLES_GLOBALES_H_


#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/log.h>

t_log *logs;
t_config *config;

int socketKernel;


int conectar_Kernel();
void enviar_archivo_al_kernel(char* archivo);
void liberar_estructuras();
int archivo_de_configuracion_valido();


#endif /* VARIAABLES_GLOBALES_H_ */
