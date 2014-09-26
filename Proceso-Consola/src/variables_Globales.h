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
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

#define PACKAGESIZE 1024
#define IP_CONEXION "127.0.0.1"
#define PUERTO_CONEXION "6668"



typedef struct
{
	int puerto_kernel;
	char* ip_kernel;
} t_configuracion;


t_configuracion configuracion;

t_configuracion levantarArchivoDeConfiguracion();


t_log *logs;
t_config *config;




int socketKernel;


int conectar_Kernel();
void enviar_archivo_al_kernel(char* archivo);
void liberar_estructuras();
int archivo_de_configuracion_valido();
int conectar_cliente();
void cerrarSocket(int socketKernel);


#endif /* VARIAABLES_GLOBALES_H_ */
