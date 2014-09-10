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

#define EXIT_ATTEMP 1

int main(void) {

	t_log* logs;
	logs = log_create("log","prueba.c",1,LOG_LEVEL_TRACE);

	char texto_entrada[20];
	t_comando opcion;

	puts("Hola!");
	puts("Bienvenido a la consola del MSP!");
	puts("Ingrese un comando o 'help' para ver la lista.");
	printf("\n> ");

	scanf("%[^\n]",&texto_entrada);

	opcion = buscar_comando(texto_entrada);
	char** argumento = string_split(texto_entrada," ");

//	ESTO ES DE TEST
	printf("[TEST] Esa es la opcion: %d.\n\n",opcion);
	int base=0;
//  -----------

	switch(opcion){
	case CREAR_SEGMENTO:

//		int base = crear_agregar_segmento(texto_a_numero(argumento[2]),texto_a_numero(argumento[3]));
		log_info(logs,string_from_format("Se creo el segmento: %d, con base: %d, tamanio: %d",texto_a_numero(argumento[2]),base,texto_a_numero(argumento[3])));
		puts("Segmento creado");
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
		return EXIT_ATTEMP;
	default:
		log_error(logs,string_from_format("Se ingreso una operacion invalida: %s",texto_entrada));
		puts("\nHa ingresado una opcion no valida.");
		puts("Ingrese 'help' para conocer los comandos.");
		break;
	}


	log_destroy(logs);
	return EXIT_SUCCESS;
}

t_comando buscar_comando(char* texto){
	char** split = string_split(texto," ");

//	ESTO ES DE TEST
	printf("\n[TEST] Ingreso:\n");
	int i=0;
	while(split[i]!=NULL){
		printf("- %s\n",split[i]);
		i++;
	}
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

int texto_a_numero(char* texto){
	int resultado=0;
	int numero;
	int mult=1;
	int i=0;
	while(texto[i]!='\0'){
		i++;
	}
	i--;
	while(i!=-1){
		numero = (texto[i]-48)*mult;
		resultado = resultado + numero;
		mult=mult*10;
		i--;
	}
	return resultado;
}
