/*
 * instrucciones_CPU.c
 *
 *  Created on: 30/09/2014
 *      Author: utnso
 */

#include "cpu.h"
#include "instrucciones_CPU.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Instrucciones de la CPU////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_LOAD(int32_t *Registro,int32_t Numero){
	*Registro = Numero;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_GETM(int32_t *Registro1,int32_t *Registro2){

	*Registro1 = *Registro2;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void instruccion_SETM(int32_t numero,int32_t registro1,int32_t registro2){




}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_MOVR(int32_t *Registro1,int32_t *Registro2){


	*Registro1 = *Registro2;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_ADDR(int32_t *Registro1,int32_t *Registro2){

	A = *Registro1 + *Registro2;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_SUBR(int32_t *Registro1,int32_t *Registro2){


	A = *Registro1 - *Registro2;


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_MULR(int32_t *Registro1,int32_t *Registro2){


	A = *Registro1 * *Registro2;


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void instruccion_MODR(int32_t *Registro1,int32_t *Registro2){


	A = *Registro1 % *Registro2;


}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_DIVR(int32_t *Registro1,int *Registro2){

	if(*Registro2 == 0){

		EFLAG = ZERO_DIV;

	}else{

		A = *Registro1 / *Registro2;

	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_INCR(int32_t *Registro1){

	*Registro1 = *Registro1 + 1;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_DECR(int32_t *Registro1){


	*Registro1 = *Registro1 - 1;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_COMP(int32_t *Registro1,int32_t *Registro2){


	if(*Registro1 == *Registro2){

		A = 1;
	}else{

		A = 0 ;

	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_CGEQ(int32_t *Registro1,int32_t *Registro2){



	if(*Registro1 >= *Registro2){

		A = 1;
	}else{

		A = 0;

	}


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_CLEQ(int32_t *Registro1,int32_t *Registro2){



	if(*Registro1 <= *Registro2){

		A = 1;

	}else{

		A = 0;

	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_GOTO(int32_t *Registro){

		program_counter =aumentarProgramCounter(TCB->base_segmento_codigo,*Registro);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_JMPZ(int32_t Direccion){

	if(A == 0){

		program_counter = Direccion;

	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_JPNZ(int32_t Direccion){


	if(A != 0){

		program_counter = Direccion;

		}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_INTE(int32_t Direccion){

	systemCall = 1; //Se produjo un llamado al sistema

	cargar_registros_TCB();

	// Envio TCB
	t_contenido mensaje_para_mandar_TCB;
	memset(mensaje_para_mandar_TCB,0,sizeof(t_contenido));
	strcpy(mensaje_para_mandar_TCB,string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_segmento_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E));
	enviarMensaje(newFD,CPU_TO_KERNEL_END_PROC,mensaje_para_mandar_TCB,logs);



}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_MALC(){

	t_contenido mensaje_para_la_msp;
	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	strcpy(mensaje_para_la_msp, string_from_format("[%d,%d]",TCB->pid,A));
	enviarMensaje(socketMSP,CPU_TO_MSP_CREAR_SEGMENTO,mensaje_para_la_msp,logs);

	t_contenido  mensaje_para_recibir_info_de_la_msp;
	recibirMensaje(socketMSP,mensaje_para_recibir_info_de_la_msp,logs);

	int32_t direccion = atoi(mensaje_para_recibir_info_de_la_msp);

	log_info(logs,"el valor de la direccion %d",direccion);

	A = direccion;

	if(direccion == EXIT_FAILURE){

		// Envio TCB
		t_contenido mensaje_para_mandar_TCB;
		memset(mensaje_para_mandar_TCB,0,sizeof(t_contenido));
		strcpy(mensaje_para_mandar_TCB,string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_segmento_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E));
		enviarMensaje(newFD,CPU_TO_KERNEL_END_PROC_ERROR,mensaje_para_mandar_TCB,logs);
	}




}
/////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_FREE(){

	t_contenido mensaje_para_la_msp;
	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	strcpy(mensaje_para_la_msp, string_from_format("[%d,%d]",TCB->pid,A));
	enviarMensaje(socketMSP,CPU_TO_MSP_DESTRUIR_SEGMENTO,mensaje_para_la_msp,logs);


	log_info(logs,"entro al FREE JAJAJAJAJAJAJ");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_INNN(){

	t_contenido mensaje_para_la_msp;
	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	enviarMensaje(newFD,CPU_TO_KERNEL_BLOQUEO_INNN,mensaje_para_la_msp,logs);

	t_contenido mensaje_para_recibir;
	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	recibirMensaje(newFD,mensaje_para_recibir,logs);

	char** vector = string_get_string_as_array(mensaje_para_recibir);
	int32_t valor = atoi(vector[0]);

	A = valor;

	log_info(logs,"el valor de A es %d",A);


}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_INNC(){

		t_contenido mensaje_para_la_msp;
		memset(mensaje_para_la_msp,0,sizeof(t_contenido));
		strcpy(mensaje_para_la_msp, string_from_format("[%d]",B));
		enviarMensaje(newFD,CPU_TO_KERNEL_BLOQUEO_INNC,mensaje_para_la_msp,logs);

		t_contenido mensaje_para_recibir;
		memset(mensaje_para_la_msp,0,sizeof(t_contenido));
		recibirMensaje(newFD,mensaje_para_recibir,logs);

		printf("el valor es %s",mensaje_para_recibir);

		t_contenido mensaje_para_msp;
		memset(mensaje_para_msp,0,sizeof(t_contenido));
		strcpy(mensaje_para_msp,string_from_format("[%d,%d,%s,%d]",A,B,(char*)mensaje_para_recibir,TCB->pid));
		enviarMensaje(socketMSP,CPU_TO_MSP_GUARDA_CADENA,mensaje_para_msp,logs);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_OUTN(){

	t_contenido mensaje_para_la_msp;
	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	strcpy(mensaje_para_la_msp,string_from_format("[%d]",A));
	enviarMensaje(newFD,CPU_TO_KERNEL_BLOQUEO_OUTN,mensaje_para_la_msp,logs);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_CREA(){

	registro_TCB *tcb_HIJO = malloc(sizeof(registro_TCB));

	tcb_HIJO->pid = A;
	tcb_HIJO->tid = TCB->pid;
	tcb_HIJO->puntero_instruccion = B;

	t_contenido mensaje_tcb_hijo_para_kernel;
	memset(mensaje_tcb_hijo_para_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_tcb_hijo_para_kernel, string_from_format("[%d,%d,%d]",tcb_HIJO->pid,tcb_HIJO->tid,tcb_HIJO->puntero_instruccion));

	enviarMensaje(newFD,CPU_TO_KERNEL_NEW_CPU_CONNECTED,mensaje_tcb_hijo_para_kernel,logs);

	free(tcb_HIJO);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_JOIN(){

	t_contenido mensaje_para_la_msp;
	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	strcpy(mensaje_para_la_msp, string_from_format("[%d]",B));
	enviarMensaje(newFD,CPU_TO_MSP_INDICEYLINEA,mensaje_para_la_msp,logs);


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_BLOK(){


}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void instruccion_PUSH(int32_t *Registro,int32_t numero){

		int32_t auxiliar_numero;
		int32_t potencia = numero * 8;

		log_info(logs,"la potencia es %d",potencia);

		int32_t _calcular_potencia(int32_t potencia){

			int32_t aux = 1;
			int32_t valor_posta = 1;

			while(aux <= potencia){

				valor_posta = valor_posta * 2;

				aux++;


			}

			if(potencia == 1){

				valor_posta = 2;

			}

			return valor_posta;
		}

		int32_t numero_potencia = _calcular_potencia(potencia) ;

		auxiliar_numero = *Registro - numero_potencia;

		log_info(logs,"el valor de la potencia es %d y del numero final es %d", numero_potencia,auxiliar_numero);

		t_contenido mensaje_para_guardar_numero_stack;
		memset(mensaje_para_guardar_numero_stack,0,sizeof(t_contenido));
		strcpy(mensaje_para_guardar_numero_stack,string_from_format("[%d,%d,%d]",TCB->pid,TCB->cursor_stack,auxiliar_numero));

		enviarMensaje(socketMSP,CPU_TO_MSP_PUSH,mensaje_para_guardar_numero_stack,logs);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void instruccion_TAKE(int32_t *Registro,int32_t numero){

		t_contenido mensaje_para_desapilar_el_stack;
		memset(mensaje_para_desapilar_el_stack,0,sizeof(t_contenido));
		strcpy(mensaje_para_desapilar_el_stack, string_from_format("[%d,%d,%d]",TCB->pid,TCB->base_stack,4));
		enviarMensaje(socketMSP,CPU_TO_MSP_TAKE,mensaje_para_desapilar_el_stack,logs);

		t_contenido recibir_mensaje;
		memset(recibir_mensaje,0,sizeof(t_contenido));
		recibirMensaje(socketMSP,recibir_mensaje,logs);

		int32_t valor_recibido;
		memcpy(&valor_recibido,recibir_mensaje,4);

		int32_t auxiliar_numero;
		int32_t potencia = numero * 8;

		int32_t _calcular_potencia(int32_t potencia){

					int32_t aux = 1;
					int32_t valor_posta = 1;

					while(aux <= potencia){

						log_info(logs,"entrounpardevecesssssssssssssssssssssssssssssssss");

						valor_posta = valor_posta * 2;

						aux++;
					}

						if(potencia == 1){

							valor_posta = 2;
						}

					return valor_posta;
				}

				int32_t numero_potencia = _calcular_potencia(potencia) ;

				auxiliar_numero = valor_recibido - numero_potencia;

				log_info(logs,"el valor es %d",auxiliar_numero);

				A = auxiliar_numero;

				aumentarProgramCounter(TCB->base_stack,4);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void instruccion_SHIF(int32_t *Registro,int32_t numero){

	if(numero > 0){

	*Registro =	*Registro >> numero;

	}else{

		numero = numero * (-1);

	*Registro = *Registro << numero;

	}



}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void instruccion_XXXX(){

	systemCall = 0; //Se supone que las instrucciones privilegiadas y las que no ,terminan con la instruccion XXXX entonces lo que hago es poner la system = 0 asi la privilegiada sale del while

	log_info(logs,"el valor de A es %d",A);


	// Envio TCB
	t_contenido mensaje_para_mandar_TCB;
	memset(mensaje_para_mandar_TCB,0,sizeof(t_contenido));
	strcpy(mensaje_para_mandar_TCB,string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_segmento_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,A,B,C,D,E));
	enviarMensaje(socketKernel,CPU_TO_KERNEL_END_PROC,mensaje_para_mandar_TCB,logs);

	log_info(logs,"Bien llego hasta sigaaaaaaaan");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_WAKE(int32_t direccion){



}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cargar_registros_TCB(){

	// Cargo los registros de programacion del TCB con los valores de los registros de la CPU

		TCB->registros_de_programacion.A = A;
		TCB->registros_de_programacion.B = B;
		TCB->registros_de_programacion.C = C;
		TCB->registros_de_programacion.D = D;
		TCB->registros_de_programacion.E = E;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



