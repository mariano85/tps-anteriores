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

void instruccion_GETM(int32_t *Registro1,int32_t Registro2){

	t_contenido mensaje_para_la_msp;
	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	strcpy(mensaje_para_la_msp,string_from_format("[%d,%d,%d]",TCB->pid,Registro2,1));
	enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_la_msp,logs);

	t_contenido mensaje_para_recibir_de_la_msp;
	memset(mensaje_para_recibir_de_la_msp,0,sizeof(t_contenido));
	recibirMensaje(socketMSP,mensaje_para_recibir_de_la_msp,logs);

	int32_t valor = 0;
	memcpy(&valor,mensaje_para_recibir_de_la_msp,1);
	*Registro1 = valor;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_SETM(int32_t numero,int32_t *Registro1,int32_t *Registro2){

	int32_t direccion_logica = *Registro1;
	int32_t numero_auxiliar;

	memcpy(&numero_auxiliar,Registro2,numero);

	t_contenido mensaje_para_la_msp;
	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	strcpy(mensaje_para_la_msp, string_from_format("[%d,%d,%d,%d]",direccion_logica,numero,numero_auxiliar,TCB->pid));
	enviarMensaje(socketMSP,CPU_TO_MSP_ESCRIBIR_MEMORIA, mensaje_para_la_msp,logs);
}

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

		P =aumentarProgramCounter(M,*Registro);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_JMPZ(int32_t Direccion){

	if(A == 0){

		P = Direccion;

	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_JPNZ(int32_t Direccion){


	if(A != 0){

		P = Direccion;

		}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_INTE(int32_t Direccion){

	puts("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

	systemCall = 1; //Se produjo un llamado al sistema

//	cargar_registros_TCB();

	P = aumentarProgramCounter(P,4);

	// Envio TCB
	t_contenido mensaje_para_mandar_TCB;
	memset(mensaje_para_mandar_TCB,0,sizeof(t_contenido));
	strcpy(mensaje_para_mandar_TCB,string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,M,TCB->tamanio_segmento_codigo,P,X,S,A,B,C,D,E,Direccion));
	enviarMensaje(socketKernel,CPU_TO_KERNEL_INTERRUPCION,mensaje_para_mandar_TCB,logs);

	puts("voy a setear aux_INTE");

	aux_INTE = 1;

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

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_INNN(){

	int32_t tipo = 0;

	t_contenido mensaje_para_el_kernel;
	memset(mensaje_para_el_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_para_el_kernel, string_from_format("[%d,%d,%d]",TCB->pid,B,tipo)); //Discutir si el TCB kernel tiene otro campo PIDKERNEL deberia
	enviarMensaje(newFD,CPU_TO_KERNEL_ENTRADA_ESTANDAR,mensaje_para_el_kernel,logs);

	t_contenido mensaje_para_recibir;
	memset(mensaje_para_recibir,0,sizeof(t_contenido));
	recibirMensaje(newFD,mensaje_para_recibir,logs);

	char** vector = string_get_string_as_array(mensaje_para_recibir);
	int32_t valor = atoi(vector[0]);

	A = valor;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_INNC(){

		int32_t tipo = 1;

		t_contenido mensaje_para_kernel;
		memset(mensaje_para_kernel,0,sizeof(t_contenido));
		strcpy(mensaje_para_kernel, string_from_format("[%d,%d,%d]",TCB->pid,B,tipo));
		enviarMensaje(newFD,CPU_TO_KERNEL_ENTRADA_ESTANDAR,mensaje_para_kernel,logs);

		t_contenido mensaje_para_recibir;
		memset(mensaje_para_recibir,0,sizeof(t_contenido));
		recibirMensaje(newFD,mensaje_para_recibir,logs);

		t_contenido mensaje_para_msp;
		memset(mensaje_para_msp,0,sizeof(t_contenido));
		strcpy(mensaje_para_msp,string_from_format("[%d,%d,%s,%d]",A,B,(char*)mensaje_para_recibir,TCB->pid));
		enviarMensaje(socketMSP,CPU_TO_MSP_ESCRIBIR_MEMORIA,mensaje_para_msp,logs);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_OUTN(){

//	int32_t tipo = 0;

	t_contenido mensaje_para_la_msp;
	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	strcpy(mensaje_para_la_msp,string_from_format("[%d,%d,%d]",TCB->pid,A,B));
	enviarMensaje(socketMSP,CPU_TO_KERNEL_SALIDA_ESTANDAR,mensaje_para_la_msp,logs);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Faltan campos
void instruccion_CREA(){

	registro_TCB *tcb_HIJO = malloc(sizeof(registro_TCB));

	TCB->cantidad_de_hijos_creados = TCB->cantidad_de_hijos_creados + 1;
	TCB->contador_de_hijos_activos = TCB->contador_de_hijos_activos + 1;

	tcb_HIJO->pid = TCB->pid;
	tcb_HIJO->tid = TCB->cantidad_de_hijos_creados;
	tcb_HIJO->puntero_instruccion = B;
	tcb_HIJO->pid = A;

	tcb_HIJO->base_segmento_codigo = TCB->base_segmento_codigo;

	tcb_HIJO->tamanio_segmento_codigo = TCB-> tamanio_segmento_codigo;
	tcb_HIJO->tamanio_segmento_stack = TCB->tamanio_segmento_stack;

	tcb_HIJO->indicador_modo_kernel = 0;

	A = tcb_HIJO->tid ;

	//int32_t tamanio_datos_viejo_stack_padre = TCB->cursor_stack -

	t_contenido mensaje_para_la_msp;
	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	strcpy(mensaje_para_la_msp, string_from_format("[%d,%d]",tcb_HIJO->pid,tcb_HIJO->tamanio_segmento_stack));
	enviarMensaje(socketMSP,CPU_TO_MSP_CREAR_SEGMENTO,mensaje_para_la_msp,logs);

	t_contenido mensaje_de_la_msp;
	memset(mensaje_de_la_msp,0,sizeof(t_contenido));
	recibirMensaje(socketMSP,mensaje_de_la_msp,logs);

	tcb_HIJO->base_stack = atoi(mensaje_de_la_msp);
	int tamanioUsado = S - X;
	tcb_HIJO->cursor_stack = aumentarProgramCounter(tcb_HIJO->base_stack, tamanioUsado);

	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	strcpy(mensaje_para_la_msp, string_from_format("[%d,%d,%d]",tcb_HIJO->pid,X,tamanioUsado));
	enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_la_msp,logs);


	memset(mensaje_de_la_msp,0,sizeof(t_contenido));
	recibirMensaje(socketMSP,mensaje_de_la_msp,logs);

	memset(mensaje_para_la_msp,0,sizeof(t_contenido));
	strcpy(mensaje_para_la_msp, string_from_format("[%d,%d,%d,%d]",tcb_HIJO->base_stack,tamanioUsado,(void*)mensaje_de_la_msp,tcb_HIJO->pid));
	enviarMensaje(socketMSP,CPU_TO_MSP_ESCRIBIR_MEMORIA,mensaje_para_la_msp,logs);

	//FALTA ATRAPAR EL ERROR



	t_contenido mensaje_tcb_hijo_para_kernel;
	memset(mensaje_tcb_hijo_para_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_tcb_hijo_para_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",tcb_HIJO->pid,tcb_HIJO->tid,tcb_HIJO->indicador_modo_kernel,M,TCB->tamanio_segmento_codigo,tcb_HIJO->puntero_instruccion,X,S,A,B,C,D,E));


	enviarMensaje(socketKernel,CPU_TO_KERNEL_CREAR_HILO,mensaje_tcb_hijo_para_kernel,logs);

	free(tcb_HIJO);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void instruccion_OUTC(){

	int32_t tipo = 1;

		t_contenido mensaje_para_la_msp;
		memset(mensaje_para_la_msp,0,sizeof(t_contenido));
		strcpy(mensaje_para_la_msp,string_from_format("[%d,%d,%d]",TCB->pid,A,tipo));
		enviarMensaje(newFD,CPU_TO_KERNEL_SALIDA_ESTANDAR,mensaje_para_la_msp,logs);

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
void instruccion_PUSH(int32_t Registro,int32_t numero){

		int32_t a = 0;
		memcpy(&a,&Registro,numero);

		log_info(logs,"El valor para la msp es %d y el corrimiento es %d, el Registro %d vale",a,numero,Registro);

		t_contenido mensaje_para_guardar_numero_stack;
		memset(mensaje_para_guardar_numero_stack,0,sizeof(t_contenido));
		strcpy(mensaje_para_guardar_numero_stack,string_from_format("[%d,%d,%d,%d]",S,4,a,TCB->pid));

		enviarMensaje(socketMSP,CPU_TO_MSP_ESCRIBIR_MEMORIA,mensaje_para_guardar_numero_stack,logs);


		S = aumentarProgramCounter(S,4);
		estado_registros();

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void instruccion_TAKE(int32_t *Registro,int32_t numero){

		int32_t tamanioUsadoStack = S - X;
		int32_t bytesAAgregarParaNuevoCursor = tamanioUsadoStack - sizeof(int32_t);
		int32_t nuevoCursor = aumentarProgramCounter(X,bytesAAgregarParaNuevoCursor);

		S = nuevoCursor;

		t_contenido mensaje_para_desapilar_el_stack;
		memset(mensaje_para_desapilar_el_stack,0,sizeof(t_contenido));
		strcpy(mensaje_para_desapilar_el_stack, string_from_format("[%d,%d,%d]",TCB->pid,S,4));
		enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_desapilar_el_stack,logs);

		t_contenido recibir_mensaje;
		memset(recibir_mensaje,0,sizeof(t_contenido));
		recibirMensaje(socketMSP,recibir_mensaje,logs);

		int32_t valor_recibido = 0;
		memcpy(&valor_recibido,recibir_mensaje,4);

		memset(Registro,0,sizeof(int32_t));

		memcpy(Registro,&valor_recibido,numero);

		estado_registros();

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void instruccion_SHIF(int32_t *Registro,int32_t numero){

	if(numero > 0){

	log_info(logs,"El valor antes es %d",Registro);

	*Registro =	*Registro >> numero;

	log_info(logs,"El valor despues es %d",Registro);

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



