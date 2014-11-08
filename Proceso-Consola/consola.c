/*
 * cliente.c
 *
 *  Created on: 02/11/2014
 *      Author: utnso
 */

#include "consola.h"

int main(int argc, char *argv[]){

	if (argc < 2) {
		fprintf(stderr, "Uso: programa archivoBESO\n");
		return EXIT_FAILURE;
	}

	loadConfig();

	int32_t id_proceso = process_getpid();
	int32_t id_hilo = process_get_thread_id();

	system("clear");
	log_info(LOGGER
		, "************** Programa BESO %s (PID: %d) (TID: %d)***************\n"
		, argv[1], id_proceso, id_hilo);

	size_t tamanioArchivo = 0;
	FILE* entrada = NULL;

	if ((entrada = fopen(argv[1], "r")) == NULL ) {
		perror(argv[1]);
		return EXIT_FAILURE;
	}

	char *besoCode = getCodigoBESO(&tamanioArchivo, entrada);
	printf("tenemos codigo, che? %s", besoCode);

	socketKernel = conectarAServidor(getIpKernel(), getPuertoKernel());

	if (socketKernel == -1) {
		printf("Imposible conectar\n\n");
		return EXIT_FAILURE;
	}

	hacerHandshakeConKernel(socketKernel);
	enviarCodigoBESO(id_proceso, id_hilo, tamanioArchivo, besoCode);
	char *mensaje = string_from_format("se envia el codigo de la consola con pid %d y tid %d", id_proceso, id_hilo);

	fclose(entrada);
	return EXIT_SUCCESS;
}

void loadConfig()
{
	CONFIG = config_create(PROGRAMA_CONF_PATH);

	if (CONFIG->properties->elements_amount == 0) {
		printf("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION\n( Don't PANIC! Si estas por consola ejecuta: ln -s ../%s %s )\n\n"
				, PROGRAMA_CONF_PATH
				, PROGRAMA_CONF_PATH);
		config_destroy(CONFIG);
		exit(-1);
	}

	if (!config_has_property(CONFIG, IP_SERVER_KEY)) {
		puts("ERROR! FALTA EL PUERTO DEL SERVER!");
		config_destroy(CONFIG);
		exit(-1);
	}

	if (!config_has_property(CONFIG, PORT_SERVER_KEY)) {
		puts("ERROR! FALTA EL PUERTO DEL SERVER!");
		config_destroy(CONFIG);
		exit(-1);
	}

	LOGGER = log_create(PROGRAMA_LOG_PATH, "Proceso Consola", true,
			LOG_LEVEL_DEBUG);
	log_info(LOGGER, "ip y puerto del server: '%s', '%d'", getIpKernel(),
			getPuertoKernel());

}

char *getCodigoBESO(size_t *tam_archivo, FILE* entrada)
{
	fseek(entrada, 0L, SEEK_END);
	*tam_archivo = ftell(entrada);

	char * literal = (char*) calloc(1, *tam_archivo);
	fseek(entrada, 0L, 0L);

	fgets(literal, *tam_archivo, entrada);
	return literal;
}

char *getIpKernel()
{
	return config_get_string_value(CONFIG, IP_SERVER_KEY);
}

int32_t getPuertoKernel()
{
	return config_get_int_value(CONFIG, PORT_SERVER_KEY);
}

void hacerHandshakeConKernel(int32_t socket)
{
	/*	printf("Envio Flag de handshake al kernel \n");*/

	enviarMensaje(socket, CON_TO_KRN_HANDSHAKE, "Test message", LOGGER);

	t_contenido mensaje;

	/*"Aviso al kernel que le voy a pasar el codigo AnSISOP y espero el ok para comenzar transaccion!"*/
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, mensaje);

	t_header header; // = recibirMensaje(socket,mensaje,loggerPrograma);

	if ((header = recibirMensaje(socket, mensaje, LOGGER))
			== ERR_CONEXION_CERRADA) {
		log_info(LOGGER,
				"El Kernel se fue! Entonces cierro mi conexion y aborto la ejecución");
		exit(EXIT_FAILURE);
	}
}

bool enviarCodigoBESO(int32_t pid, int32_t tid, size_t tamanio, char* codigoBESO)
{
	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));

	enviarMensaje(socketKernel, CON_TO_KRN_CODE, string_from_format("[%d,%d,%d]", pid, tid, tamanio), LOGGER);
	recibirMensaje(socketKernel, mensaje, LOGGER);

	memset(mensaje, 0, sizeof(t_contenido));

	enviar(socketKernel, codigoBESO, tamanio);
	recibirMensaje(socketKernel, mensaje, LOGGER);

	enviarMensaje(socketKernel, CODE_FIN_LINEAS, codigoBESO, LOGGER);

	return 1;
}
