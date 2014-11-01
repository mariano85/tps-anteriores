/*
 * consola-msp.c
 *
 *  Created on: 30/10/2014
 *      Author: utnso
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <commons/string.h>
#include <commons/log.h>
#include "msp.h"

#define EXIT_ATTEMPT 1
#define TAM_ENTRADA 60

int consola_msp() {


	//t_comando opcion;

	printf("MSP - CONSOLA\n");
	printf("Para ver la lista de comandos, escriba 'ayuda'.\n");


	while(true)
	{
		printf("\n> ");

		char* buffer = malloc(30);

		scanf("%[^\n]", buffer);
		while(getchar()!='\n');

		puts("texto entrada");
		printf("%s\n", buffer);

		if (buscarComando(buffer) == EXIT_FAILURE)
		{
			puts("Error");
		}

		free(buffer);

	}

	return EXIT_SUCCESS;
}

int buscarComando(char* buffer)
{
	char** substrings = string_split((char*)buffer, " ");

	//Armo los comandos

	if ((string_equals_ignore_case(substrings[0], "ayuda")) && (substrings[1] == NULL))
	{
		printf("AYUDA\n");
		printf("Los comandos disponibles son:\n");
		printf("\tcrear segmento [PID] [tamaño]\n");
		printf("\tdestruir segmento [PID] [dirección base]\n");
		printf("\tescribir memoria [PID] [dirección base] [bytes a escribir] [tamaño]\n");
		printf("\tleer memoria [PID] [dirección base] [tamaño]\n");
		printf("\ttabla de segmentos\n");
		printf("\ttabla de paginas [PID]\n");
		printf("\tlistar marcos\n");

		return EXIT_SUCCESS;
	}

	if (string_equals_ignore_case(substrings[0], "tabla"))
	{
		if ((substrings[1] == NULL) || (!string_equals_ignore_case(substrings[1], "de")) || (substrings[2] == NULL) || (!string_equals_ignore_case(substrings[2], "paginas")) || (substrings[3] == NULL) || (substrings[4] != NULL ))
		{
			if((substrings[1] == NULL) || (!string_equals_ignore_case(substrings[1], "de")) || (substrings[2] == NULL) || (!string_equals_ignore_case(substrings[2], "segmentos")) || (substrings[3] != NULL))
			{
				return EXIT_FAILURE;
			}
			else
			{
				printf("La instruccion es %s %s %s\n", substrings[0], substrings[1], substrings[2]);

				tablaSegmentos();
				return EXIT_SUCCESS;

			}

			return EXIT_FAILURE;
		}

		printf("El comando es %s %s %s %d\n", substrings[0], substrings[1], substrings[2], atoi(substrings[3]));

		return EXIT_SUCCESS;
	}


	if ((string_equals_ignore_case(substrings[0], "crear")))
	{
		if((substrings[1] == NULL) || (!string_equals_ignore_case(substrings[1], "segmento")) || (substrings[2] == NULL) || (substrings[3] == NULL) || (substrings[4] != NULL))
		{
			return EXIT_FAILURE;
		}

		int pid = atoi(substrings[2]);
		int tamanio = atoi(substrings[3]);

		printf("La instruccion %s %s %d %d\n", substrings[0], substrings[1], pid, tamanio);

		return EXIT_SUCCESS;
	}

	if ((string_equals_ignore_case(substrings[0], "destruir")))
	{
		if((substrings[1] == NULL) || (!string_equals_ignore_case(substrings[1], "segmento")) || (substrings[2] == NULL) || (substrings[3] == NULL) || (substrings[4] != NULL))
		{

			return EXIT_FAILURE;
		}

		int pid = atoi(substrings[2]);
		int base = atoi(substrings[3]);

		printf("La instruccion %s %s %d %d\n", substrings[0], substrings[1], pid, base);


		return EXIT_SUCCESS;
	}

	if ((string_equals_ignore_case(substrings[0], "escribir")))
	{
		if((substrings[1] == NULL) || (!string_equals_ignore_case(substrings[1], "memoria")) || (substrings[2] == NULL) || (substrings[3] == NULL) || (substrings[4] == NULL) || (substrings[5] == NULL) || (substrings[6] != NULL))
		{

			return EXIT_FAILURE;
		}

		int pid = atoi(substrings[2]);
		uint32_t direccion = atoi(substrings[3]);
		int tamanio = atoi(substrings[5]);

		printf("La instruccion %s %s %d %d %s %d\n", substrings[0], substrings[1], pid, direccion, substrings[4], tamanio);


		return EXIT_SUCCESS;
	}


	if ((string_equals_ignore_case(substrings[0], "leer")))
	{
		if((substrings[1] == NULL) || (!string_equals_ignore_case(substrings[1], "memoria")) || (substrings[2] == NULL) || (substrings[3] == NULL) || (substrings[4] == NULL) || (substrings [5] != NULL))
		{
			return EXIT_FAILURE;
		}

		int pid = atoi(substrings[2]);
		uint32_t direccion = atoi(substrings[3]);
		int tamanio = atoi(substrings[5]);

		printf("La instruccion %s %s %d %d %d\n", substrings[0], substrings[1], pid, direccion, tamanio);

		return EXIT_SUCCESS;
	}

	if ((string_equals_ignore_case(substrings[0], "listar")))
	{
		if((substrings[1] == NULL) || (!string_equals_ignore_case(substrings[1], "marcos")) || (substrings[2] != NULL))
		{
			free(substrings);

			return EXIT_FAILURE;
		}

		printf("La instruccion %s %s\n", substrings[0], substrings[1]);

		free(substrings);

		return EXIT_SUCCESS;
	}


	free(substrings);

	return EXIT_FAILURE;


}






