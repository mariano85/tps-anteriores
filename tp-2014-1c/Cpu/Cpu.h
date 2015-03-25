
#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>

#ifdef COMPILAR_DESDE_CONSOLA

/*Commons includes*/
#include <Commons/commons/collections/list.h>
#include <Commons/commons/config.h>
#include <Commons/commons/sockets.h>
#include <Commons/commons/log.h>
#include <Commons/commons/process.h>


/*Parser includes*/
#include <Parser/metadata_program.h>
#include <Parser/parser.h>

#else

/*Commons includes*/
#include "commons/collections/list.h"
#include "commons/config.h"
#include "commons/sockets.h"
#include "commons/log.h"
#include "commons/process.h"

/*Parser includes*/
#include "metadata_program.h"
#include <parser.h>

#endif



AnSISOP_funciones functions;
AnSISOP_kernel kernel_functions;

/*
 * Estructura de PCB.
 * Tamanio del contexto actual. Cantidad de variables existentes
 * En el contexto de ejecución actual. Sirve para recrear el Diccionario de variables
 * al reanudar la ejecución de un programa.
 * Cuando un Programa se vuelve a planificar y debe reanudar su ejecución en una CPU, necesita
 * conocer la CANTIDAD DE VARIABLES LOCALES que debe leer del segmento de Stack.
 */
typedef struct _t_pcb {
	int32_t contextoActual_size;
	int32_t	pId;                 //Identificador único del Programa en el sistema
	int32_t	programCounter;      //Número de la próxima instrucción a ejecutar
	int32_t indiceEtiquetas_size;//Cantidad de bytes que ocupa el Índice de etiquetas
	int32_t	indiceEtiquetas;     //Dirección del primer byte en la UMV del Índice de Etiquetas
	int32_t	indiceCodigo;        //Dirección del primer byte en la UMV del Índice de Código
	int32_t	segmentoCodigo;      //Dirección del primer byte en la UMV del segmento de código
	int32_t	cursorStack;         //Dirección del primer byte en la UMV del Contexto de Ejecución Actual
	int32_t	segmentoStack;       //Direccion del primer byte del segmento stack
} t_pcb;

typedef struct s_var_Ansisop {
	char Id[30];
	int32_t Valor;
}t_var_Ansisop;


void ejecutarSentencia(char* sentencia);
void rutinasSeniales(int senial);
char* FillNumberWithZero(char* number, int32_t fullSize);




#endif //CPU_H_
