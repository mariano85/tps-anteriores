/*
 * manejo_TCB.c
 *
 *  Created on: 08/10/2014
 *      Author: utnso
 */


#define MODO_KERNEL 1
#define MODO_USUARIO 0

typedef struct registroPCB{
    int pid;

	int tid;

	int indicador_modo_kernel;
	int base_segmento_codigo;
	int tamanio_indice_codigo ;
	int indice_codigo;
	int program_counter;
	int puntero_instruccion;
	int base_stack;
	int cursor_stack;

	int reg_programacion;


}t_tcb;
