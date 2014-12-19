/*
 * cliente.c
 *
 *  Created on: 02/11/2014
 *      Author: utnso
 */

#include "consola.h"

bool loTermino = false;

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

	socketKernel = conectarAServidor(getIpKernel(), getPuertoKernel());

	if (socketKernel == -1) {
		printf("Imposible conectar\n\n");
		return EXIT_FAILURE;
	}

	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));

	hacerHandshakeConKernel(socketKernel);

	enviarCodigoBESO(argv[1], besoCode, tamanioArchivo);
	fclose(entrada);

	while(!loTermino){
		t_header header = recibirMensaje(socketKernel, mensaje, LOGGER);

		if(header == ERR_CONEXION_CERRADA){
			log_info(LOGGER,
					"El Kernel se fue! Entonces cierro mi conexion y aborto la ejecución");
			return EXIT_FAILURE;
		} else

		if(header == KERNEL_TO_PRG_NO_MEMORY){
			log_info(LOGGER , "***ERROR*** La MSP no pudo reservar los segmentos por no tener memoria disponible");
			close(socketKernel);
			loTermino = true;
		} else

		if(header == KERNEL_TO_PRG_END_PRG){
			log_info(LOGGER,"Mi ejecución ha finalizado según correctamente! Mis registros quedaron con estos valores");
			mostrar_registros(mensaje);
			close(socketKernel);
			loTermino = true;
		} else

		if(header == KERNEL_TO_CONSOLA_ENTRADA_ESTANDAR){

			entrada_estandar(mensaje);

		} else

		if(header == KERNEL_TO_CONSOLA_SALIDA_ESTANDAR){


			salida_estandar(mensaje);
		} else

		if(header == MSP_TO_CPU_DIRECCION_INVALIDA){

			log_info(LOGGER,"***ERROR*** La direccion ingresada es invalida");
			close(socketKernel);
			loTermino = true;
		} else

		if(header == MSP_TO_CPU_VIOLACION_DE_SEGMENTO){

			log_info(LOGGER,"***ERROR*** Se produjo una violacion de segmento");
			close(socketKernel);
			loTermino = true;

		} else

		if(header == MSP_TO_CPU_PID_INVALIDO){

			log_info(LOGGER,"***ERROR*** El PID ingresado es invalido");
			close(socketKernel);
			loTermino = true;

		} else

		if(header == MSP_TO_CPU_MEMORIA_INSUFICIENTE){

			log_info(LOGGER,"***ERROR*** El PID ingresado es invalido");
			close(socketKernel);
			loTermino = true;

		} else

		if( header == MSP_TO_CPU_TAMANIO_NEGATIVO){

			log_info(LOGGER,"***ERROR*** El tamanio ingresado es negativo");
			close(socketKernel);
			loTermino = true;

		} else

		if(header == MSP_TO_CPU_SEGMENTO_EXCEDE_TAMANIO_MAXIMO){

			log_info(LOGGER,"***ERROR*** El segmento excede el tamanio maximo");
			close(socketKernel);
			loTermino = true;

		} else

		if(header == MSP_TO_CPU_PID_EXCEDE_CANT_MAXIMA_DE_SEGMENTOS){

			log_info(LOGGER,"***ERROR*** El PID excede la cantidad maxima de segmentos");
			close(socketKernel);
			loTermino = true;

		}

	}

	return EXIT_SUCCESS;
}


void mostrar_registros(char* mensaje){
	char** array = string_get_string_as_array(mensaje);
	log_info(LOGGER,"\n\tA: %d\n\tB: %d\n\tC: %d\n\tD: %d\n\tE: %d\n\t", atoi(array[8]), atoi(array[9]), atoi(array[10]), atoi(array[11]), atoi(array[12]));
}

void salida_estandar(char* mensaje) {

		char** array = string_get_string_as_array(mensaje);

		int32_t tipo = atoi(array[2]);

		if(tipo == 0){
			int32_t numero = atoi(array[1]);
			printf("\n*****IMPRIMO EN PANTALLA UN VALOR QUE ME ENVIA EL KERNEL*****\n Valor: %d\n\n",numero);

		}else{

			printf("\n*****IMPRIMO EN PANTALLA UN VALOR QUE ME ENVIA EL KERNEL*****\n Valor: %s\n\n",array[1]);

		}

}


void entrada_estandar(char* mensaje){
	char** array = string_get_string_as_array(mensaje);
	int32_t tipo = atoi(array[2]);
	int32_t tamanio = atoi(array[1]);

	if(tipo == 1){

		char* cadena = malloc(tamanio);

		printf("*****INGRESE UNA CADENA POR FAVOR*****");
		scanf("%s",cadena);

		t_contenido mensaje_para_el_kernel;
		memset(mensaje_para_el_kernel,0,sizeof(t_contenido));
		strcpy(mensaje_para_el_kernel,cadena);
		enviarMensaje(socketKernel,CONSOLA_ENVIAR_MENSAJE_KERNEL,mensaje_para_el_kernel,LOGGER);

		free(cadena);
	}else{

		int32_t valor = 0;

		printf("*****INGRESE UN VALOR NUMERICO POR FAVOR*****");
		scanf("%d",&valor);

		t_contenido mensaje_para_el_kernel;
		memset(mensaje_para_el_kernel,0,sizeof(t_contenido));
		strcpy(mensaje_para_el_kernel, string_from_format("[%d]",valor));
		enviarMensaje(socketKernel,CONSOLA_ENVIAR_MENSAJE_KERNEL,mensaje_para_el_kernel,LOGGER);


	}


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

	fread(literal, *tam_archivo, 1, entrada);
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

	enviarMensaje(socket, PRG_TO_KRN_HANDSHAKE, "Test message", LOGGER);

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


bool enviarCodigoBESO(char* nombreBESO, char* codigoBESO, size_t tamanio)
{
	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));

	enviarMensaje(socketKernel, PRG_TO_KRN_CODE, string_from_format("[%s,%d]", nombreBESO, tamanio), LOGGER);
	recibirMensaje(socketKernel, mensaje, LOGGER);

	memset(mensaje, 0, sizeof(t_contenido));

	enviar(socketKernel, codigoBESO, tamanio);

	return 1;
}
