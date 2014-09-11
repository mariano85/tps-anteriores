/*
 ============================================================================
 Name        : proyectoPractica.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "cpu.h"

///////////////////////////////////////////////////////////////
//////////////////////////Main principal ////////////////////////
///////////////////////////////////////////////////////////////////

#include "variablesGlobales_Funciones.h"

int main(int argc, char** argv) {

	argc = 2; //Le paso un parametro asi no pincha
	**argv = 'a';

	logs = log_create("log", "CPU.c", 1, LOG_LEVEL_TRACE);

	if (argc < 2){
			log_error(logs, "No se pasaron parametros.");
			log_destroy(logs);
			return 0;
		}

		A = malloc(sizeof(int));
		B = malloc(sizeof(int));
		C = malloc(sizeof(int));
		D = malloc(sizeof(int));
		E = malloc(sizeof(int));

	void inicializar_CPU();


	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////// Falta verificar que el archivo de configuracion sea valido //////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////


	/*Al iniciar,intentara conectarse a los procesos kernel y msp,en caso de no poder
conctarse,abortara su ejecucion informando por pantalla y el log del motivo*/

//	socketKernel = conectar_kernel();
	//socketMSP =conectar_MSP();
	//	if(socketKernel < 0 || socketMSP < 0){
	//		log_error(logs, "El programa tuvo que finalizar insatisfactoriamente porque fallo alguna conexion");
			//liberar_estructuras();
	//		return 0;}



 // pthread_create(&pthread_Consola,NULL,manejo_consola,NULL);

  	  	LOAD(A,12);
		printf("%d\n",*A);
		GETM(D,A);
		printf("%d\n",*D);
		ADDR(D,A);
		printf("%d\n",*D);


		free(A);
		free(B);
		free(C);
		free(D);
		free(E);

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

int conectar_kernel(){



	return 0; //Por ahora devuelvo 0 porque todavia no defini lo que hace esta funcion


}



int conectar_MSP(){


	return 0; //Por ahora devuelvo 0 porque no defini lo que hace esta funcion

}


void inicializar_CPU(){





}






