/*
 * programa.c
 *
 *  Created on: 17/04/2014
 *      Author: utnso
 */

#include "proceso_consola.h"

int main(int argc, char *argv[]) {

	if(argc < 2){
		fprintf(stderr, "Uso: programa archivoAnsisop\n");
		return EXIT_FAILURE;
	}

	iniciarPrograma();

	FILE *entrada;

	int32_t id_proceso = getpid();
	system("clear");
	log_info(LOGGER, "************** Programa Ansisop %s (PID: %d) ***************\n",argv[1],id_proceso);

	char * NOM_ARCHIVO = argv[1];

	if ((entrada = fopen(NOM_ARCHIVO, "r")) == NULL){
		perror(NOM_ARCHIVO);
		return EXIT_FAILURE;
	}

	char *literal = getLiteralFromFile(entrada);

	puts("aca deberia mandarlo al Kernel");

	// obtuve un socket programando yo mismo!
	int32_t socketServer = -1;

	// me puedo conectar de esta manera? ESTA FUNCION NO SIRVE!!!
	if(	( conectar(getIpKernel(), getPuertoKernel(), &socketServer) ) == -1){
		printf("Imposible conectar\n\n");
		return EXIT_FAILURE;
	}

	printf("ip: %s; puerto: %d; socket del servidor: %d\n", getIpKernel(), getPuertoKernel(), socketServer);

	enviar(socketServer, literal, strlen(literal) + 1);
	enviar(socketServer, "\n************* PRUEBA SERVIDOR*******", strlen("\n************* PRUEBA SERVIDOR*******") + 1);

	free(literal);
	fclose(entrada);
	close(socketServer);
	puts("eso es todo el archivo");

	return EXIT_SUCCESS;
}

char* getLiteralFromFile(FILE* entrada){

	char linea[LONG_MAX_LINEA];
	fseek(entrada, 0L, SEEK_END);
	uint32_t tam_archivo = ftell(entrada);
	char * literal = calloc(1, tam_archivo);
	fseek(entrada, 0L, 0L);

	while (fgets(linea, LONG_MAX_LINEA, entrada) != NULL){
		strcat(literal, linea);
	}

	return literal;
}

char* getBytesFromFile(FILE* entrada){
	fseek(entrada, 0L, SEEK_END);
	uint32_t tam_archivo = ftell(entrada);
	char * literal = calloc(1, tam_archivo);
	fseek(entrada, 0L, 0L);

	fgets(literal, LONG_MAX_LINEA, entrada);
	return literal;
}

void iniciarPrograma(){
	CONFIG = config_create(PROGRAMA_CONF_PATH);

	if (CONFIG->properties->elements_amount == 0) {
		printf("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION %s \n", PROGRAMA_CONF_PATH);
		perror("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION\n( Don't PANIC! Si estas por consola ejecuta: ln -s /home/utnso/workspace/programa/programa.conf programa.conf )\n\n");
		config_destroy(CONFIG);
		exit(-1);
	}

	if(!config_has_property(CONFIG, "IP_KERNEL")){
		puts("ERROR! FALTA EL PUERTO DEL SERVER!");
		config_destroy(CONFIG);
		exit(-1);
	}

	if(!config_has_property(CONFIG, "PUERTO_KERNEL")){
		puts("ERROR! FALTA EL PUERTO DEL SERVER!");
		config_destroy(CONFIG);
		exit(-1);
	}

	LOGGER = log_create(PROGRAMA_LOG_PATH, "Proceso Consola", true, LOG_LEVEL_DEBUG);
	log_info(LOGGER, "ip y puerto del server: '%s', '%d'", getIpKernel(), getPuertoKernel() );

}

char *getIpKernel(){
	return config_get_string_value(CONFIG, IP_SERVER_KEY);
}

int32_t getPuertoKernel(){
	return config_get_int_value(CONFIG, PORT_SERVER_KEY);
}

void finalizarPrograma(){
	config_destroy(CONFIG);
	log_destroy(LOGGER);
}
