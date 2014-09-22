/*
 ============================================================================
 Name        : cpu.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "variablesGlobales_Funciones.h"

TCB p1;

int main(int argc, char** argv) {

/*	argc = 2; //Le paso un parametro asi no pincha
	**argv = 'a';

	logs = log_create("log", "CPU.c", 1, LOG_LEVEL_TRACE);

	if (argc < 2){
			log_error(logs, "No se pasaron parametros.");
			log_destroy(logs);
			return 0;
		}*/

		A = malloc(sizeof(int));
		B = malloc(sizeof(int));
		C = malloc(sizeof(int));
		D = malloc(sizeof(int));
		E = malloc(sizeof(int));
		EFLAG = malloc(sizeof(char));


	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////// Falta verificar que el archivo de configuracion sea valido //////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////


	/*Al iniciar,intentara conectarse a los procesos kernel y msp,en caso de no poder
conctarse,abortara su ejecucion informando por pantalla y el log del motivo*/

	socketKernel = conectar_kernel();
	socketMSP =conectar_MSP();
		if(socketKernel < 0 || socketMSP < 0){
	//		log_error(logs, "El programa tuvo que finalizar insatisfactoriamente porque fallo alguna conexion");
		//	liberar_estructuras();
			return 0;}


		//Creo el hilo para atender la consola

//	p_HILO = pthread_create(&pthread_Consola,NULL,(void*)manejo_consola,NULL);

	p_HILO = pthread_create(&pthread_Consola,NULL,(void*)inicializar_CPU_conexion_kernel,NULL);
	pthread_join(pthread_Consola,NULL);

	printf("El valor del hilo CONSOLA es %d\n",p_HILO);



		LOAD(A,12);
		printf("%d\n",*A);
		GETM(D,A);
		printf("%d\n",*D);
		ADDR(D,A);
		printf("%d\n",*D);
		SUBR(D,A);
		printf("%d\n",*D);
		MULR(A,D);
		printf("%d\n",*A);
		MODR(A,D);
		printf("%d\n",*A);

		free(A);
		free(B);
		free(C);
		free(D);
		free(E);
		free(EFLAG);



		return EXIT_SUCCESS;
}



///////////////////////////////////////////////////////////////////
//////////////////////// Funciones ///////////////////////////////////
///////////////////////////////////////////////////////////////////

void LOAD(int *Registro,int Numero){



	*Registro = Numero;

}

void GETM(int *Registro1,int *Registro2){

	*Registro1 = *Registro2;

}

void ADDR(int *Registro1,int *Registro2){

	*Registro1 = *Registro1 + *Registro2;

}

void SUBR(int *Registro1,int *Registro2){


	*Registro1 = *Registro1 - *Registro2;


}

void MULR(int *Registro1,int *Registro2){


	*Registro1 = *Registro1 * *Registro2;


}

void MODR(int *Registro1,int *Registro2){


	*Registro1 = *Registro1 % *Registro2;


}


void DIVR(int *Registro1,int *Registro2){

	if(*Registro2 == 0){

		EFLAG = "ZERO_DIV";

	}else{

		*Registro1 = *Registro1 / *Registro2;

	}

}

void INCR(int *Registro1){


	*Registro1 = *Registro1 + 1;



}

void DECR(int *Registro1){


	*Registro1 = *Registro1 - 1;

}

void COMP(int *Registro1,int *Registro2){

	int aux;

	if(*Registro1 == *Registro2){

		aux = 1;
	}else{

		aux = 0 ;

	}

	*Registro1 = aux;

}

void CGEQ(int *Registro1,int *Registro2){

	int aux;

	if(*Registro1 >= *Registro2){

		aux = 1;
	}else{

		aux = 0;

	}

	*Registro1 = aux;
}

void CLEQ(int *Registro1,int *Registro2){

	int aux;

	if(*Registro1 <= *Registro2){

		aux = 1;
	}else{

		aux = 0;

	}

	*Registro1 = aux;
}

void GOTO(int *Registro){

	*PC = *Registro;

}

void manejo_consola(){

	while(1){
			consola();
			printf("===========================================================\n");
		}
}


void consola(){

		puts("Ingrese una instruccion o 'help' para ver la lista.");
		printf("\n> ");

		scanf("%s\n",&texto_entrada);

		switch(texto_entrada){

		case 'L' :

			scanf("%d",&aux);

			LOAD(A,aux);

			printf("Tu Load es %d",*A);
		break;



}
	}

int conectar_MSP(){


	return 0; //Por ahora devuelvo 0 porque no defini lo que hace esta funcion

}

int conectar_kernel(){



	return 0; //Por ahora devuelvo 0 porque todavia no defini lo que hace esta funcion


}


int inicializar_CPU_conexion_kernel(){

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

		getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE


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


		bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
		freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar


		listen(listenningSocket, BACKLOG);

		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);

		int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);


		int status = 1;



		puntero_estructura_a_recibir = malloc(sizeof(TCB));



		printf("CPU conectado a kernel, esperando por mensajes. Esperando mensajes:\n");

		while (status != 0){
			status = recv(socketCliente, (void*) puntero_estructura_a_recibir,sizeof(TCB), 0);
			p1.PID = *puntero_estructura_a_recibir;
			p1.TID = *(puntero_estructura_a_recibir + 4);

			status = 0; // Lo pongo porque lo repite nose porque no sale

			printf("%d",p1.PID);
			printf("%d",p1.TID);
			//		if (status != 0) printf("%s", package);

		}

		close(socketCliente);
		close(listenningSocket);



		return 0;
	}

