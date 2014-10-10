/*
 ============================================================================
 Name        : consola_msp.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "commons/string.h"
#include "commons/log.h"
#include "consola_msp.h"
#include "../msp.h"

#define EXIT_ATTEMPT 1
#define TAM_ENTRADA 60


int consola_msp() {

	t_log* logs;
	logs = log_create("log","consola_msp",1,LOG_LEVEL_TRACE);


	t_comando opcion;

	puts("-------------------------------------------------");
	printf("\n");
	puts("Hola!");
	puts("Bienvenido a la consola del MSP!");


while(true){ /*while.................................................................................*/


	puts("\n-------------------------------------------------");
	printf("\n");
	puts("Ingrese un comando o 'help ' para ver la lista.");
	printf("\n> ");


	char* texto_entrada = malloc(TAM_ENTRADA);
	fgets(texto_entrada,TAM_ENTRADA,stdin);


	opcion = buscar_comando(texto_entrada);
	char** argumento = string_split(texto_entrada," ");


//	ESTO ES DE TEST
//	printf("[TEST] Esa es la opcion: %d.\n\n",opcion);
//  -----------

	uint32_t base;

	switch(opcion){
	case CREAR_SEGMENTO:
		if(argumento[3]==NULL){
			puts("\nDebe ingresar PID y el tamanio.");
			break;
		}
		if(atoi(argumento[3])==0){
			puts("\nDebe ingresar un tamanio distinto de 0.");
			break;
		}
		base = crearSegmento(atoi(argumento[2]),atoi(argumento[3]));
		printf("Segmento creado con base en: %d\n", base);
		printf("PID: %d, TAM: %d\n", atoi(argumento[2]), atoi(argumento[3]));
		break;


	case DESTRUIR_SEGMENTO:
		puts("Comando para destruir segmento.");
		break;


	case ESCRIBIR_MEMORIA:
		puts("Comando para escribir memoria.");
		break;


	case LEER_MEMORIA:
		puts("Comando para leer memoria.");
		break;


	case TABLA_SEGMENTOS:
		puts("Comando para tabla de segmentos.");
		break;


	case TABLA_PAGINAS:
		puts("Comando para tabla de paginas.");
		break;


	case LISTAR_MARCOS:
		puts("Comando para lista de marcos.");
		break;


	case ERROR:
		log_error(logs,string_from_format("Se ingreso una operacion invalida: %s",texto_entrada));
		puts("\nHa ingresado una opcion no valida.");
		puts("Ingrese 'help' para conocer los comandos.");
		break;


	case HELP:
		printf("\nEstos son los comandos disponibles en la consola:\n\n");
		printf("CREAR SEGMENTO: \n    > crear segmento (PID) (TAMANIO)\n");
		printf("DESTRUIR SEGMENTO: \n    > destruir segmento (PID) (DIRECCION BASE)\n");
		printf("ESCRIBIR MEMORIA: \n    > escribir memoria (PID) (DIRECCION) (TAMANIO) (TEXTO)\n");
		printf("LEER MEMORIA: \n    > leer memoria (PID) (DIRECCION) (TAMANIO)\n");
		printf("IMPRIMIR TABLA DE SEGMENTOS: \n    > imprimir tabla de segmentos\n");
		printf("IMPRIMIR TABLA DE PAGINAS: \n    > imprimir tabla de paginas (PID)\n");
		printf("IMPRIMIR LISTA DE MARCOS: \n    > imprimir lista de marcos\n");
		printf("SALIR: \n    > exit\n");
		break;


	case EXIT:
		log_warning(logs,"Se ha cerrado la consola MSP.");
		return EXIT_ATTEMPT;


	default:
		log_error(logs,string_from_format("Se ingreso una operacion invalida: %s",texto_entrada));
		puts("\nHa ingresado una opcion no valida.");
		puts("Ingrese 'help' para conocer los comandos.");
		break;
	}

	free(texto_entrada);
	fflush(stdin);

} /*while............................................................................................*/


	log_destroy(logs);
	return EXIT_SUCCESS;

}









t_comando buscar_comando(char* texto){
	char** split = string_split((char*)texto, " ");

//	ESTO ES DE TEST
//	printf("\n[TEST] Ingreso:\n");
//	int i=0;
//	while(split[i]!=NULL){
//		printf("- %s\n",split[i]);
//		i++;
//	}
//	-------

// Usando la funcion de comparar cadenas devolver el comando correspondiente.
// bool	string_equals_ignore_case(char * actual, char * expected);

	if(string_equals_ignore_case(split[0],"crear")){
		if(string_equals_ignore_case(split[1],"segmento")){
			return CREAR_SEGMENTO;
		}
		return ERROR;
	}

	if(string_equals_ignore_case(split[0],"destruir")){
		if(string_equals_ignore_case(split[1],"segmento")){
			return DESTRUIR_SEGMENTO;
		}
		return ERROR;
	}

	if(string_equals_ignore_case(split[0],"escribir")){
		if(string_equals_ignore_case(split[1],"memoria")){
			return ESCRIBIR_MEMORIA;
		}
		return ERROR;
	}

	if(string_equals_ignore_case(split[0],"leer")){
		if(string_equals_ignore_case(split[1],"memoria")){
			return LEER_MEMORIA;
		}
		return ERROR;
	}

	if(string_equals_ignore_case(split[0],"imprimir")){
		if(string_equals_ignore_case(split[1],"tabla")){
			if(string_equals_ignore_case(split[2],"de")){
				if(string_equals_ignore_case(split[3],"segmentos")){
					return TABLA_SEGMENTOS;
				}
				if(string_equals_ignore_case(split[3],"paginas")){
					return TABLA_PAGINAS;
				}
				return ERROR;
			}
			return ERROR;
		}

		if(string_equals_ignore_case(split[1],"lista")){
			if(string_equals_ignore_case(split[2],"de")){
				if(string_equals_ignore_case(split[3],"marcos")){
					return LISTAR_MARCOS;
				}
				return ERROR;
			}
			return ERROR;
		}
		return ERROR;
	}



	if(string_equals_ignore_case(split[0],"help")){
			return HELP;
		}

	if(string_equals_ignore_case(split[0],"exit")){
			return EXIT;
		}
	return ERROR;
}
