/*
 * funciones_CPU.c
 *
 *  Created on: 01/10/2014
 *      Author: utnso
 */


#include "funciones_CPU.h"



/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

	void iniciarPrograma(){
		config = config_create(PROGRAMA_CONF_PATH);

		if (config->properties->elements_amount == 0) {
			printf("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION %s \n", PROGRAMA_CONF_PATH);
			perror("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION\n( Don't PANIC! Si estas por consola ejecuta: ln -s /home/utnso/workspace/programa/programa.conf programa.conf )\n\n");
			config_destroy(config);
			exit(-1);
		}

		if(!config_has_property(config,IP_KERNEL)){
			puts("ERROR! FALTA EL PUERTO DEL KERNEL!");
			config_destroy(config);
			exit(-1);
		}

		if(!config_has_property(config,IP_MSP)){
			puts("ERROR! FALTA EL PUERTO DE LA MSP!");
			config_destroy(config);
			exit(-1);
		}

		if(!config_has_property(config,PUERTO_KERNEL)){
			puts("ERROR! FALTA EL PUERTO DEL KERNEL!");
			config_destroy(config);
			exit(-1);
		}

		if(!config_has_property(config,PUERTO_MSP)){

			puts("ERROR! FALTA EL PUERTO DE LA MSP");
			config_destroy(config);
			exit(-1);
		}

		if(!config_has_property(config,RETARDO)){

			puts("ERROR! FALTA EL RETARDO");
			config_destroy(config);
			exit(-1);

		}

/*		sem_init(&mutex_A, 0, 1);
		sem_init(&mutexREADY, 0, 1);
		sem_init(&mutexEXEC, 0, 1);
		sem_init(&mutexEXIT, 0, 1);
		sem_init(&mutexBLOCK, 0, 1);*/







		log_info(logs, "Archivo de configuracion levantado correctamente");

	}

//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

	void liberar_estructuras_CPU(){

			dictionary_destroy(diccionarioDeVariables);
			config_destroy(config);
			log_destroy(logs);


	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

void buscador_de_instruccion(char* instruccion){

	//#1 LOAD : carga en el registro el numero dado

	if(string_equals_ignore_case(LOAD,instruccion)){

			int32_t registro =solicitar_registro();

			int32_t numero = solicitar_numero();

			instruccion_LOAD(registro,numero);
		}

	//#2 GETM : Obtiene el valor de memoria apuntado por el segundo registro.El valor obtenido lo asigna en el primer registro

	if(string_equals_ignore_case(GETM,instruccion)){

			int32_t registro1 = solicitar_registro();

			int32_t registro2 = solicitar_registro();

			instruccion_GETM(registro1,registro2);
		}

	//#3 SETM : Pone tantos bytes desde el segundo registro, hacia la memoria apuntada por el primer registro

	if(string_equals_ignore_case(SETM,instruccion)){

			int32_t numero = solicitar_numero();

			int32_t registro1 = solicitar_registro();

			int32_t registro2 = solicitar_registro();

			instruccion_SETM(numero,registro1,registro2);

		}

	//#4 MOVR : copia el valor del segundo registro al primer registro

	if(string_equals_ignore_case(MOVR,instruccion)){

		int32_t registro1 = solicitar_registro();

		int32_t registro2 = solicitar_registro();

		instruccion_MOVR(registro1,registro2);
	}

	//#5 ADDR : Sume el primer registro con el segundo registro. El resultado de la operacion se almacena en el registro A

	if(string_equals_ignore_case(ADDR,instruccion)){

		int32_t registro1 = solicitar_registro();

		int32_t registro2 = solicitar_registro();

		instruccion_ADDR(registro1,registro2);
	}

	//#6 SUBR : Resta el primer registro con el segundo registro. El resultado lo almacena en el primer registro A

	if(string_equals_ignore_case(SUBR,instruccion)){

			int32_t registro1 = solicitar_registro();

			int32_t registro2 = solicitar_registro();

			instruccion_SUBR(registro1,registro2);
		}

	//#7 MULR : Multiplica el primer registro con el segundo registro. El resultado de la operacion se almacena en el registro A

	if(string_equals_ignore_case(MULR,instruccion)){

			int32_t registro1 = solicitar_registro();

			int32_t registro2 = solicitar_registro();

			instruccion_MULR(registro1,registro2);

	}

	//#8 MODR : Obtiene el resto de la division del primer registro con el segundo registro. El resultado de la operacion se almacen en el registro A

	if(string_equals_ignore_case(MODR,instruccion)){

			int32_t registro1 = solicitar_registro();

			int32_t registro2 = solicitar_registro();

			instruccion_MODR(registro1,registro2);

	}

	//#9 DIVR : Divide el primer registro con el segundo registro. El resultado de la operacion se almacena en el registro A; a menos que el segundo operando sea 0, en cuyo caso se asigna el flag de ZERO_DIV y no se hace la operacion

	if(string_equals_ignore_case(DIVR,instruccion)){

			int32_t registro1 = solicitar_registro();

			int32_t registro2 = solicitar_registro();

			instruccion_DIVR(registro1,registro2);

	}

	//#10 INCR : Incrementa en una unidad al registro

	if(string_equals_ignore_case(INCR,instruccion)){

			int32_t registro1 = solicitar_registro();

			instruccion_INCR(registro1);

	}

	//#11 DECR : Decrementa en una unidad al registro

	if(string_equals_ignore_case(DECR,instruccion)){

			int32_t registro1 = solicitar_registro();

			instruccion_DECR(registro1);

	}

	//#12 COMP : Compara si el primer registro es igual al segundo. De ser verdadero, se almacena el valor 1.De lo contrario el valor 0.El resultado de la operacion se almacena en el registro A

	if(string_equals_ignore_case(COMP,instruccion)){

			int32_t registro1 = solicitar_registro();

			int32_t registro2 = solicitar_registro();

			instruccion_COMP(registro1,registro2);

	}

	//#13 CGEQ : Compara si el primer registro es mayor o igual al segundo. De ser verdadero, se almacena el valor 1.Delo contrario el valor 0.El resultado de la operacion se almacen en el registro A

	if(string_equals_ignore_case(CGEQ,instruccion)){

			int32_t registro1 = solicitar_registro();

			int32_t registro2 = solicitar_registro();

			instruccion_CGEQ(registro1,registro2);

	}

	//#14 CLEQ : Compara si el primer registro es menor o igual al segundo. De ser verdadero, se almacena el valor 1.Delo contrario el valor 0.El resultado de la operacion se almacen en el registro A

	if(string_equals_ignore_case(CLEQ,instruccion)){

			int32_t registro1 = solicitar_registro();

			int32_t registro2 = solicitar_registro();

			instruccion_CLEQ(registro1,registro2);
	}

	//#15 GOTO : Altera el flujo de ejecucion para ejecutar la instruccion apuntada por el registro. El valor es el desplazamiento desde el inicio del programa

	if(string_equals_ignore_case(GOTO,instruccion)){

			int32_t registro1 = solicitar_registro();

			instruccion_GOTO(registro1);

	}

	//#16 JMPZ : Altera el flujo de ejecucion, solo si el valor del registro A es cero,para ejecutar la instruccion apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.

	if(string_equals_ignore_case(JMPZ,instruccion)){

			int32_t direccion = solicitar_direccion();

			instruccion_JPMZ(direccion);

	}

	//#17 JPNZ : Altera el flujo de ejecucion, solo si el valor del registro A no es cero,para ejecutar la instruccion apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.

	if(string_equals_ignore_case(JPNZ,instruccion)){

			int32_t direccion = solicitar_direccion();

			instruccion_JPNZ(direccion);

	}

	//#18 INTE[Direccion]
	//Interrumpe la ejecución del programa para ejecutar la rutina del kernel que se encuentra en la
	//posición apuntada por la direccion. El ensamblador admite ingresar una cadena indicando el
	//nombre, que luego transformará en el número correspondiente. Los posibles valores son
	//“MALC”, “FREE”, “INNN”, “INNC”, “OUTN”, “OUTC”, “BLOK”, “WAKE”, “CREA” y “JOIN”. Invoca al
	//servicio correspondiente en el proceso Kernel. Notar que el hilo en cuestión debe bloquearse
	//tras una interrupción.

	if(string_equals_ignore_case(INTE,instruccion)){

				int32_t direccion = solicitar_direccion();

				instruccion_INTE(direccion);

	}


}

int32_t solicitar_registro(){


	return 0;

}


int32_t solicitar_numero(){



	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
char* recibir_Instruccion(){

	char* puntero;

	puntero = malloc(sizeof(char));



	return puntero;

}

//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void cargar_diccionario(){



}

int32_t solicitar_direccion(){



	return 0;

}
