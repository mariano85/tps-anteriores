/*
 ============================================================================
 Name        : cpu.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "cpu.h"
#include <commons/config.h>
TCB p1;

int main(int argc, char** argv) {

//	configuracion = levantarArchivoDeConfiguracion();

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

	/*Al iniciar,intentara conectarse a los procesos kernel y msp,en caso de no poder
conctarse,abortara su ejecucion informando por pantalla y el log del motivo*/

	socketKernel = conectar_kernel();
	socketMSP =conectar_MSP();
		if(socketKernel < 0 || socketMSP < 0){
	//		log_error(logs, "El programa tuvo que finalizar insatisfactoriamente porque fallo alguna conexion");
		//	liberar_estructuras();
			return 0;}


	p_HILO = pthread_create(&pthread_Consola,NULL,(void*)inicializar_CPU_conexion_kernel,NULL);
	pthread_join(pthread_Consola,NULL);

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

t_configuracion levantarArchivoDeConfiguracion()
{
	//Levanto archivo de configuracion
		char* ip_kernel;
		int puerto_kernel;
		char* ip_msp;
		int puerto_msp;
		int retardo;

		t_configuracion configuracion;


		/*Compruebo si existe el archivo (tal vez no sea la manera más
		 * eficiente).
		 */
		FILE* file;

		file = fopen("configuracion", "r");

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

		char* path = "configuracion";

		config = config_create(path);

		//Pido los valores de configuracion y los vuelco en una estructura
		ip_kernel = config_get_string_value(config, "IP_KERNEL");
		ip_msp = config_get_string_value(config, "IP_MSP");
		puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
		puerto_msp = config_get_int_value(config, "PUERTO_MSP");
		retardo = config_get_int_value(config, "RETARDO");

		configuracion.ip_kernel = ip_kernel;
		configuracion.puerto_kernel = puerto_kernel;
		configuracion.puerto_msp = puerto_msp;
		configuracion.retardo = retardo;
		configuracion.ip_msp = ip_msp;

		free(config);

		printf("ip kernel: %s\n", configuracion.ip_kernel);
		printf("ip msp: %s\n", configuracion.ip_msp);
		printf("puerto kernel: %d\n", configuracion.puerto_kernel);
		printf("puerto msp: %d\n", configuracion.puerto_msp);
		printf("retardo: %d\n", configuracion.retardo);


	//termine de levantar archivo de configuracion


	return configuracion;
}



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

