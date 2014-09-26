/*
 ============================================================================
 Name        : Proceso-Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "variables_Globales.h"



int main(int argc, char **argv){

/*	argc = 2;

	logs = log_create("log", "program.c", 1, LOG_LEVEL_TRACE);
		if (argc < 2){
			log_error(logs, "No se paso el archivo con el codigo BESO");
			liberar_estructuras();
			return 0;
		} */

/*		if (!archivo_de_configuracion_valido()){
				log_error(logs, "Archivo de configuracion incompleto");
			liberar_estructuras();
				return 0;
			}



	//	socketKernel = conectar_Kernel(); //No funca
			if (socketKernel < 0){
				log_error(logs, "El programa finalizo, no se pudo conectar con el kernel");
				liberar_estructuras();
				return 0;}

	//	log_info(logs, "El proceso se conecto correctamente con el kernel");

//	enviar_archivo_al_kernel(argv[1]); // Mando el archivo con el codigo BESO
// Tengo una duda de como se levanta el archivo con el codigo BESO, P R E G U N T A R


		cerrarSocket(socketKernel);

		liberar_estructuras();*/


	//configuracion = levantarArchivoDeConfiguracion();

	conectar_cliente();



	return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////Funciones/////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

t_configuracion levantarArchivoDeConfiguracion()
{
 	int puerto_kernel;
	char* ip_kernel;
	t_configuracion configuracion;


	/*Compruebo si existe el archivo (tal vez no sea la manera más
	 * eficiente).
	 */
	FILE* file;

	file = fopen("ESO_CONFIG", "r");

	if(file == NULL )
	{
		puts("El archivo no existe!");
		exit(0);

	}
	else
	{
		puts("El archivo existe!");
		fclose(file);
	}


	//Abro el archivo de configuracion
	t_config *config;

	config = malloc(sizeof(t_config));

	char* path = "ESO_CONFIG";

	config = config_create(path);

	//Pido los valores de configuracion y los vuelco en una estructura
	puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
	ip_kernel = config_get_string_value(config, "IP_KERNEL");

	configuracion.puerto_kernel = puerto_kernel;
	configuracion.ip_kernel = ip_kernel;


	free(config);

	printf("ip kernel: %s\n", configuracion.ip_kernel);
	printf("puerto kernel: %d\n", configuracion.puerto_kernel);

	//Devuelvo la estructura cuyos campos son los parametros de configuracion
	return configuracion;
}

int conectar_Kernel(){

			char *ip = config_get_string_value(config,"IP_KERNEL");
			char *Puerto = config_get_string_value(config,"PUERTO_KERNEL");

			return conectar_cliente(ip,Puerto,logs);

}

void enviar_archivo_al_kernel(char* archivo){






}

/* Libero la estructuras utilizadas	*/

void liberar_estructuras(){
	config_destroy(config);
	log_destroy(logs);
}


/* Verifico que el archivo de configuracion sea valido */
int archivo_de_configuracion_valido(){
	if (!config_has_property(config, "IP_KERNEL"))
		return 0;
	if (!config_has_property(config, "PUERTO_KERNEL"))
		return 0;
	return 1;
}

int conectar_cliente(){


	/*
		 *  ¿Quien soy? ¿Donde estoy? ¿Existo?
		 *
		 *  Estas y otras preguntas existenciales son resueltas getaddrinfo();
		 *
		 *  Obtiene los datos de la direccion de red y lo guarda en serverInfo.
		 *
		 */
		struct addrinfo hints;
		struct addrinfo *serverInfo;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
		hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

		getaddrinfo(IP_CONEXION,PUERTO_CONEXION, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


		/*
		 * 	Ya se quien y a donde me tengo que conectar... ¿Y ahora?
		 *	Tengo que encontrar una forma por la que conectarme al server... Ya se! Un socket!
		 *
		 * 	Obtiene un socket (un file descriptor -todo en linux es un archivo-), utilizando la estructura serverInfo que generamos antes.
		 *
		 */
		int serverSocket;
		serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

		/*
		 * 	Perfecto, ya tengo el medio para conectarme (el archivo), y ya se lo pedi al sistema.
		 * 	Ahora me conecto!
		 *
		 */
		connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
		freeaddrinfo(serverInfo);	// No lo necesitamos mas

		/*
		 *	Estoy conectado! Ya solo me queda una cosa:
		 *
		 *	Enviar datos!
		 *
		 *	Vamos a crear un paquete (en este caso solo un conjunto de caracteres) de size PACKAGESIZE, que le enviare al servidor.
		 *
		 *	Aprovechando el standard immput/output, guardamos en el paquete las cosas que ingrese el usuario en la consola.
		 *	Ademas, contamos con la verificacion de que el usuario escriba "exit" para dejar de transmitir.
		 *
		 */
		int enviar = 1;
		char message[PACKAGESIZE];

		printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");

		while(enviar){
			fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
			if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
			if (enviar) send(serverSocket, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
		}


		/*
		 *	Listo! Cree un medio de comunicacion con el servidor, me conecte con y le envie cosas...
		 *
		 *	...Pero me aburri. Era de esperarse, ¿No?
		 *
		 *	Asique ahora solo me queda cerrar la conexion con un close();
		 */

		close(serverSocket);
		return 0;

		/* ADIO'! */
	}

