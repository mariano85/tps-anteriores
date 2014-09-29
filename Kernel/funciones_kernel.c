/*
 * funciones_kernel.c
 *
 *  Created on: 26/09/2014
 *      Author: utnso
 */

#include "global.h"


int inicializar_Kernel_comunicacion_CPU(){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);



	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	// if -> chequear la conexion

	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	int enviar = 1;

	p1.PID = 32;

	p1.TID = 66565;

	p1.KM = 1;


	puntero_estructura_a_mandar = malloc(12);

	memcpy(puntero_estructura_a_mandar,&p1.PID,4);
	memcpy(puntero_estructura_a_mandar + 4,&p1.TID,4);
	memcpy(puntero_estructura_a_mandar + 8,&p1.KM,4);

	printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");

	while(enviar){

		send(serverSocket,puntero_estructura_a_mandar,12,0);

		enviar = 0;

	}


	close(serverSocket);
	return 0;


}

int inicializar_Kernel_comunicacion_PROCESO_CONSOLA(){


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
		hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
		hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
		hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

		getaddrinfo(NULL, PUERTO_CONEXION, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE


		/*
		 * 	Descubiertos los misterios de la vida (por lo menos, para la conexion de red actual), necesito enterarme de alguna forma
		 * 	cuales son las conexiones que quieren establecer conmigo.
		 *
		 * 	Para ello, y basandome en el postulado de que en Linux TODO es un archivo, voy a utilizar... Si, un archivo!
		 *
		 * 	Mediante socket(), obtengo el File Descriptor que me proporciona el sistema (un integer identificador).
		 *
		 */
		/* Necesitamos un socket que escuche las conecciones entrantes */
		int listenningSocket;
		listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

		/*
		 * 	Perfecto, ya tengo un archivo que puedo utilizar para analizar las conexiones entrantes. Pero... ¿Por donde?
		 *
		 * 	Necesito decirle al sistema que voy a utilizar el archivo que me proporciono para escuchar las conexiones por un puerto especifico.
		 *
		 * 				OJO! Todavia no estoy escuchando las conexiones entrantes!
		 *
		 */
		bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
		freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

		/*
		 * 	Ya tengo un medio de comunicacion (el socket) y le dije por que "telefono" tiene que esperar las llamadas.
		 *
		 * 	Solo me queda decirle que vaya y escuche!
		 *
		 */
		listen(listenningSocket, BACKLOG);		// IMPORTANTE: listen() es una syscall BLOQUEANTE.

		/*
		 * 	El sistema esperara hasta que reciba una conexion entrante...
		 * 	...
		 * 	...
		 * 	BING!!! Nos estan llamando! ¿Y ahora?
		 *
		 *	Aceptamos la conexion entrante, y creamos un nuevo socket mediante el cual nos podamos comunicar (que no es mas que un archivo).
		 *
		 *	¿Por que crear un nuevo socket? Porque el anterior lo necesitamos para escuchar las conexiones entrantes. De la misma forma que
		 *	uno no puede estar hablando por telefono a la vez que esta esperando que lo llamen, un socket no se puede encargar de escuchar
		 *	las conexiones entrantes y ademas comunicarse con un cliente.
		 *
		 *			Nota: Para que el listenningSocket vuelva a esperar conexiones, necesitariamos volver a decirle que escuche, con listen();
		 *				En este ejemplo nos dedicamos unicamente a trabajar con el cliente y no escuchamos mas conexiones.
		 *
		 */
		struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
		socklen_t addrlen = sizeof(addr);

		int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);

		/*
		 * 	Ya estamos listos para recibir paquetes de nuestro cliente...
		 *
		 * 	Vamos a ESPERAR (ergo, funcion bloqueante) que nos manden los paquetes, y los imprimieremos por pantalla.
		 *
		 *	Cuando el cliente cierra la conexion, recv() devolvera 0.
		 */
		char package[PACKAGESIZE];
		int status = 1;		// Estructura que manjea el status de los recieve.

		printf("Cliente conectado. Esperando mensajes:\n");

		while (status != 0){
			status = recv(socketCliente, (void*) package, PACKAGESIZE, 0);
			if (status != 0) printf("%s", package);

		}

		/*
		 * 	Terminado el intercambio de paquetes, cerramos todas las conexiones y nos vamos a mirar Game of Thrones, que seguro nos vamos a divertir mas...
		 *
		 *
		 * 																					~ Divertido es Disney ~
		 *
		 */
		close(socketCliente);
		close(listenningSocket);

		/* See ya! */

		return 0;
	}
