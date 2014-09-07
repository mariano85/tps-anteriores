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

int main(int argc, char** argv) {


	logs = log_create("log", "CPU.c", 1, LOG_LEVEL_TRACE);
	config = config_create("cpu.conf");

	diccionarioDeVariables = dictionary_create();

	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////// Falta verificar que el archivo de configuracion sea valido //////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////


	/*Al iniciar,intentara conectarse a los procesos kernel y msp,en caso de no poder
conctarse,abortara su ejecucion informando por pantalla y el log del motivo*/

	socketKernel = conectar_kernel();
	socketMSP =conectar_MSP();

	if(socketKernel < 0 || socketMSP < 0){
		log_error(logs, "El programa tuvo que finalizar insatisfactoriamente porque fallo alguna conexion");
		//liberar_estructuras();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}



///////////////////////////////////////////////////////////////////
//////////////////////// Funciones ///////////////////////////////////
///////////////////////////////////////////////////////////////////


void interpretar_sentencia_ESO(int bytecode,char registro,int numero){


	switch (bytecode) {

		case 1297045074: //Si es LOAD

			registro = numero;

				break;

	}





	}


int conectar_kernel(){



	return 0; //Por ahora devuelvo 0 porque todavia no defini lo que hace esta funcion


}



int conectar_MSP(){


	return 0; //Por ahora devuelvo 0 porque no defini lo que hace esta funcion

}


