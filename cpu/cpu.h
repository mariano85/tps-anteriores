/*
 * proyectoPractica.h
 *
 *  Created on: 29/08/2014
 *      Author: utnso
 */

#ifndef PROYECTOPRACTICA_H_
#define PROYECTOPRACTICA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/bitarray.h>
#include <commons/temporal.h>
#include <commons/log.h>
#include <commons/config.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

t_log *logs;
t_config* config;
t_dictionary* diccionarioDeVariables;


int registroA;
int registroB;
int socketKernel;
int socketMSP;

#endif /* PROYECTOPRACTICA_H_ */

void interpretar_sentencia_ESO(int a,char b,int c);
int conectar_kernel();
int conectar_MSP();

