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

void instruccion_LOAD(int Registro,int Numero){


	//wait();
	Registro = Numero;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_GETM(int Registro1,int Registro2){

	Registro1 = Registro2;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_SETM(int32_t numero,int32_t registro1,int32_t registro2){




}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_MOVR(int Registro1,int Registro2){


	Registro1 = Registro2;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_ADDR(int Registro1,int Registro2){

	Registro1 = Registro1 + Registro2;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_SUBR(int Registro1,int Registro2){


	Registro1 = Registro1 - Registro2;


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_MULR(int Registro1,int Registro2){


	Registro1 = Registro1 * Registro2;


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_MODR(int Registro1,int Registro2){


	Registro1 = Registro1 % Registro2;


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_DIVR(int Registro1,int Registro2){

	if(Registro2 == 0){

		EFLAG = ZERO_DIV;

	}else{

		Registro1 = Registro1 / Registro2;

	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_INCR(int Registro1){


	Registro1 = Registro1 + 1;



}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_DECR(int Registro1){


	Registro1 = Registro1 - 1;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_COMP(int Registro1,int Registro2){

	int aux_2;

	if(Registro1 == Registro2){

		aux_2 = 1;
	}else{

		aux_2 = 0 ;

	}

	Registro1 = aux_2;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_CGEQ(int Registro1,int Registro2){

	int aux;

	if(Registro1 >= Registro2){

		aux = 1;
	}else{

		aux = 0;

	}

	Registro1 = aux;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_CLEQ(int Registro1,int Registro2){

	int aux;

	if(Registro1 <= Registro2){

		aux = 1;
	}else{

		aux = 0;

	}

	Registro1 = aux;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_GOTO(int Registro){

	program_counter = Registro;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_JPMZ(int Direccion){

	if(A == 0){

		program_counter = Direccion;

	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_JPNZ(int Direccion){


	if(A != 0){

		program_counter = Direccion;

		}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_INTE(int32_t Direccion){

	systemCall = 1; //Se produjo un llamado al sistema

	char* rutina_del_kernel = solicitar_rutina_kernel(Direccion);

	if(string_equals_ignore_case(MALC,rutina_del_kernel)){

					int32_t direccion = solicitar_direccion();

					instruccion_MALC(direccion);

	}

	if(string_equals_ignore_case(FREE,rutina_del_kernel)){

						int32_t direccion = solicitar_direccion();

						instruccion_FREE(direccion);

		}


	if(string_equals_ignore_case(INNN,rutina_del_kernel)){

						int32_t direccion = solicitar_direccion();

						instruccion_INNN(direccion);

		}

	if(string_equals_ignore_case(INNC,rutina_del_kernel)){

							int32_t direccion = solicitar_direccion();

							instruccion_INNC(direccion);

			}

	if(string_equals_ignore_case(OUTN,rutina_del_kernel)){

								int32_t direccion = solicitar_direccion();

								instruccion_OUTN(direccion);

				}

	if(string_equals_ignore_case(CREA,rutina_del_kernel)){

									int32_t direccion = solicitar_direccion();

									instruccion_CREA(direccion);

					}

	if(string_equals_ignore_case(JOIN,rutina_del_kernel)){

										int32_t direccion = solicitar_direccion();

										instruccion_JOIN(direccion);

						}

	if(string_equals_ignore_case(BLOK,rutina_del_kernel)){

											int32_t direccion = solicitar_direccion();

											instruccion_BLOK(direccion);


			}

	if(string_equals_ignore_case(WAKE,rutina_del_kernel)){

												int32_t direccion = solicitar_direccion();

												instruccion_WAKE(direccion);


				}


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_MALC(int32_t direccion){

	devolver_TCB();

	// Cargo el resto de la info del TCB y se lo paso al KERNEL

	t_contenido mensaje_para_devolverle_el_TCB_al_kernel;
	memset(mensaje_para_devolverle_el_TCB_al_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_para_devolverle_el_TCB_al_kernel,strcpy(mensaje_para_devolverle_el_TCB_al_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_indice_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E,direccion)));
	enviarMensaje(socketKernel,CPU_TO_KERNEL_INTERRUPCION_POR_MALC_BLOQUEAR_PROCESO,mensaje_para_devolverle_el_TCB_al_kernel,logs);

	//Necesito que me mande el TCB del Kernel

	recibir_el_TCB_modo_Kernel();

}
/////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_FREE(int32_t direccion){

	devolver_TCB();
	t_contenido mensaje_para_devolverle_el_TCB_al_kernel;
	memset(mensaje_para_devolverle_el_TCB_al_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_para_devolverle_el_TCB_al_kernel,strcpy(mensaje_para_devolverle_el_TCB_al_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_indice_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E,direccion)));
	enviarMensaje(socketKernel,CPU_TO_KERNEL_INTERRUPCION_POR_FREE_BLOQUEAR_PROCESO,mensaje_para_devolverle_el_TCB_al_kernel,logs);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_INNN(int32_t direccion){

	devolver_TCB();
	t_contenido mensaje_para_devolverle_el_TCB_al_kernel;
	memset(mensaje_para_devolverle_el_TCB_al_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_para_devolverle_el_TCB_al_kernel,strcpy(mensaje_para_devolverle_el_TCB_al_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_indice_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E,direccion)));
	enviarMensaje(socketKernel,CPU_TO_KERNEL_INTERRUPCION_POR_INNN_BLOQUEAR_PROCESO,mensaje_para_devolverle_el_TCB_al_kernel,logs);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_INNC(int32_t direccion){

	devolver_TCB();
	t_contenido mensaje_para_devolverle_el_TCB_al_kernel;
	memset(mensaje_para_devolverle_el_TCB_al_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_para_devolverle_el_TCB_al_kernel,strcpy(mensaje_para_devolverle_el_TCB_al_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_indice_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E,direccion)));
	enviarMensaje(socketKernel,CPU_TO_KERNEL_INTERRUPCION_POR_INNC_BLOQUEAR_PROCESO,mensaje_para_devolverle_el_TCB_al_kernel,logs);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_OUTN(int32_t direccion){

	devolver_TCB();
	t_contenido mensaje_para_devolverle_el_TCB_al_kernel;
	memset(mensaje_para_devolverle_el_TCB_al_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_para_devolverle_el_TCB_al_kernel,strcpy(mensaje_para_devolverle_el_TCB_al_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_indice_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E,direccion)));
	enviarMensaje(socketKernel,CPU_TO_KERNEL_INTERRUPCION_POR_OUTN_BLOQUEAR_PROCESO,mensaje_para_devolverle_el_TCB_al_kernel,logs);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_CREA(int32_t direccion){

	devolver_TCB();
	t_contenido mensaje_para_devolverle_el_TCB_al_kernel;
	memset(mensaje_para_devolverle_el_TCB_al_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_para_devolverle_el_TCB_al_kernel,strcpy(mensaje_para_devolverle_el_TCB_al_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_indice_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E,direccion)));
	enviarMensaje(socketKernel,CPU_TO_KERNEL_INTERRUPCION_POR_CREA_BLOQUEAR_PROCESO,mensaje_para_devolverle_el_TCB_al_kernel,logs);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_JOIN(int32_t direccion){

	devolver_TCB();
	t_contenido mensaje_para_devolverle_el_TCB_al_kernel;
	memset(mensaje_para_devolverle_el_TCB_al_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_para_devolverle_el_TCB_al_kernel,strcpy(mensaje_para_devolverle_el_TCB_al_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_indice_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E,direccion)));
	enviarMensaje(socketKernel,CPU_TO_KERNEL_INTERRUPCION_POR_JOIN_BLOQUEAR_PROCESO,mensaje_para_devolverle_el_TCB_al_kernel,logs);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_BLOK(int32_t direccion){

	devolver_TCB();
	t_contenido mensaje_para_devolverle_el_TCB_al_kernel;
	memset(mensaje_para_devolverle_el_TCB_al_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_para_devolverle_el_TCB_al_kernel,strcpy(mensaje_para_devolverle_el_TCB_al_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_indice_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E,direccion)));
	enviarMensaje(socketKernel,CPU_TO_KERNEL_INTERRUPCION_POR_BLOK_BLOQUEAR_PROCESO,mensaje_para_devolverle_el_TCB_al_kernel,logs);

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_XXXX(){

	systemCall = 0; //Se supone que las instrucciones privilegiadas y las que no terminan con la instruccion XXXX entonces lo que hago es poner la system = 0 asi la privilegiada sale del while

	devolver_TCB();
		t_contenido mensaje_para_devolverle_el_TCB_al_kernel;
		memset(mensaje_para_devolverle_el_TCB_al_kernel,0,sizeof(t_contenido));
		strcpy(mensaje_para_devolverle_el_TCB_al_kernel,strcpy(mensaje_para_devolverle_el_TCB_al_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_indice_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E)));
		enviarMensaje(socketKernel,CPU_TO_KERNEL_END_PROC,mensaje_para_devolverle_el_TCB_al_kernel,logs);

}

////////////////////////////////////////////////////////////////////////////////////////////////////

void instruccion_WAKE(int32_t direccion){

		devolver_TCB();
		t_contenido mensaje_para_devolverle_el_TCB_al_kernel;
		memset(mensaje_para_devolverle_el_TCB_al_kernel,0,sizeof(t_contenido));
		strcpy(mensaje_para_devolverle_el_TCB_al_kernel,strcpy(mensaje_para_devolverle_el_TCB_al_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_indice_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,TCB->registros_de_programacion.A,TCB->registros_de_programacion.B,TCB->registros_de_programacion.C,TCB->registros_de_programacion.D,TCB->registros_de_programacion.E,direccion)));
		enviarMensaje(socketKernel,CPU_TO_KERNEL_INTERRUPCION_POR_WAKE_BLOQUEAR_PROCESO,mensaje_para_devolverle_el_TCB_al_kernel,logs);


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char* solicitar_rutina_kernel(int32_t Direccion){








	return("HOLA");


}

/////////////////////////////////////////////////////////////////////////////////////////////////

void devolver_TCB(){

	// Cargo los registros de programacion del TCB con los valores de los registros de la CPU

		TCB->registros_de_programacion.A = A;
		TCB->registros_de_programacion.B = B;
		TCB->registros_de_programacion.C = C;
		TCB->registros_de_programacion.D = D;
		TCB->registros_de_programacion.E = E;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void recibir_el_TCB_modo_Kernel(){

	//Me preparo para recbir el TCB KERNEL y volveria a la ejecucion del programa pero como estoy en modo kernel y systemcall = 1 paso al while de abajo en cpu.c

		t_contenido mensaje_para_recibir_el_TCB_del_kernel;

		t_header mensaje_del_kernel_que_me_mando_el_TCB_modo_kernel = recibirMensaje(socketKernel,mensaje_para_recibir_el_TCB_del_kernel,logs);

		if(mensaje_del_kernel_que_me_mando_el_TCB_modo_kernel!= KERNEL_TO_CPU_TCB_MODO_KERNEL){
							log_info(logs, "Se cerró la conexion con el KERNEL");
							exit(EXIT_FAILURE);
						}
}

