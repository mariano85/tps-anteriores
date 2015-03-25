#include "Programa.h"



t_log* loggerPrograma;
char * ip;
int32_t puerto;
bool NOBRNOCOMMENTS = false;
int32_t loTermino = 0;
t_config* archivoConfig;

int32_t main(int argc, char *argv[]) {

	//Verifica si se recibio como parametro el path del config, sino aborta la ejecucion
	if (argc == 1) {
		printf("Error! No se recibió ningún script para iniciar el proceso.\nEjecución abortada.\n");
		exit(EXIT_FAILURE);
	}

	int myPID = process_getpid();

	//Crea log de proceso.
	loggerPrograma = log_create("programa.log", "Programa", true, LOG_LEVEL_DEBUG);

	//Obtiene de la variable de entorno, la ruta al config y obtiene valores del mismo.
	char * config_path = getenv("ANSISOP_PATH");
	log_info(loggerPrograma, "config_path: %s", config_path);
	cargarConfiguracion(config_path);

	//Obtengo el codigo del archivo para pasarlo al Kernel.
	char * fileCodigo = argv[1];
	log_info(loggerPrograma, "fileCodigo: %s", fileCodigo);
	char* codigo = getCodigoAnsisop(fileCodigo);

	//Primer contacto con el Kernel y retry en caso de ausencia!
	int32_t socketKernel = conectarAServidor(ip, puerto);
	while (socketKernel == EXIT_FAILURE) {
		log_info(loggerPrograma,
				"Despierten al Kernel! Se reintenta conexion en unos segundos ;) \n");
		sleep(5);
		socketKernel = conectarAServidor(ip, puerto);
	}

	hacerHandshakeConKernel(socketKernel);

	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));

	int32_t seEnvioCodigo = enviarCodigo(socketKernel, PRG_TO_KRN_CODE, string_from_format("%i", myPID), codigo, loggerPrograma, NOBRNOCOMMENTS);

	if (!seEnvioCodigo) {
		log_error(loggerPrograma, "Fallo el envio del codigo");
		return EXIT_FAILURE;
	}
	else
		log_info(loggerPrograma, "Se envió el código satisfactoriamente :)");

	while (!loTermino) {

		t_header header;

		if ((header = recibirMensaje(socketKernel, mensaje, loggerPrograma)) == ERR_CONEXION_CERRADA) {
			log_info(loggerPrograma,
					"El Kernel se fue! Entonces cierro mi conexion y aborto la ejecución");
			return EXIT_FAILURE;
		}

		else if (header == KRN_TO_PRG_IMPR_PANTALLA) {
			imprimir_en_pantalla(mensaje);
		} else if (header == KRN_TO_PRG_IMPR_VARIABLES) {
			printf("\n\tIMPRIMO VALORES FINALES DE VARIABLES:\n");
			int j = 0;
			while(j < strlen(mensaje)){
				printf("\tvariable %s = %s\n",string_substring(mensaje, j, 1),string_substring(mensaje, j+1, 4));
				j+=5;
			}
		} else if (header == KRN_TO_PRG_IMPR_IMPRESORA) {
			imprimir_en_impresora(mensaje);
		} else if (header == KRN_TO_PRG_END_PRG) {
			log_info(loggerPrograma,"Mi ejecución ha finalizado según el Kernel! Cierro el socket, no lloren por mi...ya estoy muerto :(");
			close(socketKernel);
			loTermino = 1;
		}
		else if (header == KRN_TO_PRG_NO_MEMORY) {
			log_info(loggerPrograma,"Mi ejecución ha finalizado porque el sistema no tiene suficiente memoria para soportarme(?)! Changos, maldición!");
			close(socketKernel);
			loTermino = 1;
		}

	}

	free(codigo);
	log_destroy(loggerPrograma);
	config_destroy(archivoConfig);
	return EXIT_SUCCESS;
}

/**
 * @NAME: hacerHandshakeConKernel
 * @DESC: Realiza el primer intercambio de mensajes con el proceso Kernel
 */
void hacerHandshakeConKernel(int32_t socket) {
	/*	printf("Envio Flag de handshake al kernel \n");*/

	enviarMensaje(socket, PRG_TO_KRN_HANDSHAKE, "Test message", loggerPrograma);

	t_contenido mensaje;
	t_header header; // = recibirMensaje(socket,mensaje,loggerPrograma);

	if ((header = recibirMensaje(socket, mensaje, loggerPrograma)) == ERR_CONEXION_CERRADA) {
		log_info(loggerPrograma,
				"El Kernel se fue! Entonces cierro mi conexion y aborto la ejecución");
		exit(EXIT_FAILURE);
	}
}

/**
 * @NAME: cargarConfiguracion
 * @DESC: Settea variables propias de este proceso, con datos de su archivo de configuracion
 */
void cargarConfiguracion(char* pathArchiConf) {

	if (pathArchiConf == NULL ) {
		log_error(loggerPrograma,
				"No esta declarada la variable de entorno ANSISOP_PATH - FIN DE PROGRAMA");
		log_error(loggerPrograma,
				"EJECUTAR EL COMANDO: export ANSISOP_PATH=/<ruta-a-config>/programa.config");
		exit(EXIT_FAILURE);
	}

	log_info(loggerPrograma, "Se inicializa el Programa con %s", pathArchiConf);
	archivoConfig = config_create(pathArchiConf);

	if (config_has_property(archivoConfig, "puerto")) {
		puerto = config_get_int_value(archivoConfig, "puerto");
	} else {
		log_error(loggerPrograma,
				"No se encontro la key 'puerto' en el archivo de configuracion");
		exit(EXIT_FAILURE);
	}

	if (config_has_property(archivoConfig, "ip")) {
		ip = config_get_string_value(archivoConfig, "ip");
	} else {
		log_error(loggerPrograma,
				"No se encontro la key 'ip' en el archivo de configuracion");
		exit(EXIT_FAILURE);
	}

	log_info(loggerPrograma, "Me voy a conectar a: %s:%d", ip, puerto);

	if (config_has_property(archivoConfig, "NOBRNOCOMMENTS")) {
		int32_t var = config_get_int_value(archivoConfig, "NOBRNOCOMMENTS");
		if(var){
			NOBRNOCOMMENTS = true;
		}
		else
		{
			NOBRNOCOMMENTS = false;
		}
	} else {
		log_error(loggerPrograma,
				"No se encontro la key 'NOBRNOCOMMENTS' en el archivo de configuracion");
		exit(EXIT_FAILURE);
	}

}

/**
 * @NAME: imprimir_en_pantalla
 * @DESC: Imprime en pantalla (Solicitado por el Kernel)
 */
void imprimir_en_pantalla(char* mensaje) {
	printf("\n*****IMPRIMO EN PANTALLA UN VALOR QUE ME ENVIA EL KERNEL*****\n Valor: %s\n\n", mensaje);
}

/**
 * @NAME: imprimir_en_impresora
 * @DESC: Imprime en impresora (Solicitado por el Kernel)
 */
void imprimir_en_impresora(char * mensaje) {
	printf("\n*****El Kernel me pide imprimir texto... :D ****\nEl texto es: %s\n\n", mensaje);
}

/**
 * @NAME: getCodigoAnsisop
 * @DESC: Obtiene el codigo AnSISOP desde un archivo recibido por parametro
 */
char* getCodigoAnsisop(char* path) {

	char* str = string_new();

	FILE* file = fopen(path, "r");
	char line[256];
	bool primeraLinea = true;
	printf("---------- source code ini ----------\n");
	while (fgets(line, sizeof(line), file)) {
		if (!primeraLinea) { // omito la primer linea del codigo para sacar el hashband
			printf("%s", line);
			string_append(&str, line);
		} else {
			primeraLinea = false;
		}
	}
	printf("---------- source code end ----------\n");
	fclose(file);
	return str;
}


