/*
 * global.h
 *
 *  Created on: 12/09/2014
 *      Author: utnso
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PUERTO "6667"
#define PACKAGESIZE 1024

int inicializar_Kernel_comunicacion_CPU();

#endif /* GLOBAL_H_ */
