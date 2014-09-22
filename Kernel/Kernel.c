///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Kernel De Prueba ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

// Es un kernel de prueba para crear una comunicacion con la cpu

#include "global.h"

	TCB p1;

int main(){

	p1.PID = 32;

	p1.TID = 2;

	//p_HILO = pthread_create(&pthread_Proceso_Consola,NULL,(void*)inicializar_Kernel_comunicacion_PROCESO_CONSOLA(),NULL);

	//pthread_join(pthread_Proceso_Consola,NULL);

	p_HILO_CONSOLA = pthread_create(&pthread_Proceso_Consola,NULL,(void*)inicializar_Kernel_comunicacion_CPU(),NULL);

	pthread_join(pthread_CPU,NULL);

	return 0;


}

////////////////////////////FUNCIONES KERNEL /////////////////////////////////////////////

int inicializar_Kernel_comunicacion_CPU(){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);



	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	int enviar = 1;

	puntero_estructura_a_mandar = malloc(sizeof(TCB));

	*puntero_estructura_a_mandar = p1.PID;
	*(puntero_estructura_a_mandar + 4) = p1.TID;



	printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");

	while(enviar){

		send(serverSocket,puntero_estructura_a_mandar,sizeof(TCB),0);

		enviar = 0;

	}


	close(serverSocket);
	return 0;


}

int inicializar_Kernel_comunicacion_PROCESO_CONSOLA(){


	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, PUERTO_CONEXION, &hints, &serverInfo);



	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar


	listen(listenningSocket, BACKLOG);


	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);


	int package[PACKAGESIZE];
	int status = 1;
	printf("Cliente conectado. Esperando mensajes:\n");

	while (status != 0){
		status = recv(socketCliente, (void*) package, PACKAGESIZE, 0);
		if (status != 0) printf("%d",*package);

	}


	close(socketCliente);
	close(listenningSocket);



	return 0;
}




