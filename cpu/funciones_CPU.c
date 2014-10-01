/*
 * funciones_CPU.c
 *
 *  Created on: 01/10/2014
 *      Author: utnso
 */

#include "cpu.h"
#include "funciones_CPU.h"

int conectar_MSP(){


	return 0; //Por ahora devuelvo 0 porque no defini lo que hace esta funcion

}

int conectar_kernel(){



	return 0; //Por ahora devuelvo 0 porque todavia no defini lo que hace esta funcion


}


int inicializar_CPU_conexion_kernel(){

		struct addrinfo hints;
		struct addrinfo *serverInfo;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
		hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
		hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

		getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE

		int listenningSocket;
		listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);


		bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
		freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar


		listen(listenningSocket, BACKLOG);

		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);

		int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);


		int status = 1;



		puntero_estructura_a_recibir = malloc(12);

		printf("CPU conectado a kernel, esperando por mensajes. Esperando mensajes:\n");

		while (status != 0){
			status = recv(socketCliente, (void*) puntero_estructura_a_recibir,12, 0);
			memcpy(&p1.PID,puntero_estructura_a_recibir,4);
			memcpy(&p1.TID,puntero_estructura_a_recibir + 4,4);
			memcpy(&p1.KM,puntero_estructura_a_recibir + 8,4);
			memcpy(&p1.M,puntero_estructura_a_recibir + 12,4);

			status = 0; // Lo pongo porque lo repite nose porque no sale

			printf("el PID es %d\n",p1.PID);
			printf("el TID es %d\n",p1.TID);
			printf("el KM es %d\n",p1.KM);


		}


		close(socketCliente);
		close(listenningSocket);



		return 0;

}



void iniciarPrograma(){
	config = config_create(PROGRAMA_CONF_PATH);

	if (config->properties->elements_amount == 0) {
		printf("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION %s \n", PROGRAMA_CONF_PATH);
		perror("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION\n( Don't PANIC! Si estas por consola ejecuta: ln -s /home/utnso/workspace/programa/programa.conf programa.conf )\n\n");
		config_destroy(config);
		exit(-1);
	}

	if(!config_has_property(config,IP_KERNEL)){
		puts("ERROR! FALTA EL PUERTO DEL SERVER!");
		config_destroy(config);
		exit(-1);
	}

	if(!config_has_property(config,PUERTOKERNEL)){
		puts("ERROR! FALTA EL PUERTO DEL SERVER!");
		config_destroy(config);
		exit(-1);
	}

	log_info(logs, "Archivo de configuracion levantado correctamente");

}
