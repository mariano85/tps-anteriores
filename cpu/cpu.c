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
#include "instrucciones_CPU.h"
#include "funciones_CPU.h"



int main(int argc, char *argv[]) {




	logs = log_create("log", "CPU.c", 1, LOG_LEVEL_TRACE);

	if (argc < 2){
			log_error(logs, "No se pasaron parametros.");
			log_destroy(logs);
			return 0;
		}


	iniciarPrograma();




	/*	A = malloc(sizeof(int));
		B = malloc(sizeof(int));
		C = malloc(sizeof(int));
		D = malloc(sizeof(int));
		E = malloc(sizeof(int));
		EFLAG = malloc(sizeof(char));


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
		free(EFLAG);*/



		return EXIT_SUCCESS;
}



///////////////////////////////////////////////////////////////////
//////////////////////// Funciones ///////////////////////////////////
///////////////////////////////////////////////////////////////////



