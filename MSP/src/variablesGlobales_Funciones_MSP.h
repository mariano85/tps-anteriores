/*
 * variablesGlobales_Funciones_MSP.h
 *
 *  Created on: 05/09/2014
 *      Author: utnso
 */

#ifndef VARIABLESGLOBALES_FUNCIONES_MSP_H_
#define VARIABLESGLOBALES_FUNCIONES_MSP_H_


#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////Variables Globales //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

t_log* logs;
t_config* config;
int tamanio_MSP = 419430; //Son 16777216 bytes resultado de hacer 4096 segmentos * 4096 tamaño

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////Estructuras//////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*Este struct es para crear la estructura del segmento, para mi lo que tiene es el identificador del segmento que nos sirve para
 sacar los primeros 12 bits de la direccion logica y buscar ese segmento.
El tamanio como no es fijo lo guardo de paso sirve para validar que no me pase a otro segmento */


typedef struct tablaPaginas{ 		/*Estructura tablaPaginas, esta estructura se repetira 16 veces por segmentos pues cada segmentos
											creado tendra 16 paginas*/
	int dirLogica;
	char *dirFisica;

    }tablaPaginas; 			/*Esta es una instancia de la estructura que vamos a utilizar, si no ponemos la instancia la estructura
               	   	   	   	   	   es una plantilla para crear instancias despues, por ahora lo dejamos asi despues vemos que conviene */

struct tablaPaginas arreglo[16]; // Es un arreglo de estructuras, es un arreglo de 16 posiciones de tipo tablaPaginas

typedef struct nodo_seg{	//
	int idSegmento;
	int tamanioSegmento;

	struct arreglo *puntero_paginas ; //Apunta al arreglo que contiene la estructura tablaPaginas

}nodo_seg;


////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////Declaracion De Funciones ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void inicializar_MSP(int tamanioMSP);
void inicializar_Configuracion();


#endif /* VARIABLESGLOBALES_FUNCIONES_MSP_H_ */
