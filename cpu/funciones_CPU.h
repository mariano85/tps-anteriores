/*
 * funciones_CPU.h
 *
 *  Created on: 01/10/2014
 *      Author: utnso
 */

#ifndef FUNCIONES_CPU_H_
#define FUNCIONES_CPU_H_

#include "cpu.h"
#include "instrucciones_CPU.h"
#include "funciones_CPU.h"

t_configuracion levantarArchivoDeConfiguracion();

void inicializar_Configuracion();



void manejo_consola(); //Voy a crear una mini consola para probar las cosas

void consola();

int inicializar_CPU_conexion_kernel();

void iniciarPrograma();


#endif /* FUNCIONES_CPU_H_ */
