/*
 * planificador.c
 *
 *  Created on: 17/10/2014
 *      Author: utnso
 */



#include "kernel.h"
t_log* kernelLog;

void* planificador(t_loaderThread *loaderThread){


	int32_t pid = 1234;
	int32_t program_counter = 1048576;
	int32_t QUANTUM =10;
	int32_t MODO = 1;


		for(;;) {


				log_info(logKernel,"Me meti en el planificador");

				// Le mando el TCB a CPU

				t_contenido mensaje_1;
				memset(mensaje_1, 0, sizeof(t_contenido));
				strcpy(mensaje_1, string_from_format("[%d,%d,%d,%d]", pid,program_counter,QUANTUM,MODO));
				enviarMensaje(socket_cpu, KERNEL_TO_CPU_TCB, mensaje_1, logKernel);
				log_info(logKernel, "Se envía un TCB al CPU libre elegido");

				//////////////////////////////////////////
				///////////////////////////////////////////////
				////////////////READY////////////////////////
				////////READY/////////////////////////READY///
				////////////////////////READY////////////////

				//Espero a que cpu me devuelva el TCB

				t_contenido mensaje_de_la_cpu;
				memset(mensaje_de_la_cpu,0,sizeof(t_contenido));
				t_header header_recibido_de_la_CPU = recibirMensaje(socket_cpu,mensaje_de_la_cpu,logKernel);

				if(header_recibido_de_la_CPU == CPU_TO_KERNEL_FINALIZO_QUANTUM_NORMALMENTE){

					char** array = string_get_string_as_array(mensaje_de_la_cpu);

					//Aca hay que guardarlo en un estructura TCB y mandarlo a la cola de ready pero no se cual estructura usan te lo dejo a vos lean
					//MUESTRO LO QUE DEBERIA LLEGARTE IGUAL

					log_info(logKernel,"PID ES %d, Program_counter es %d ,modo es %d",atoi(array[0]),atoi(array[1]),atoi(array[2]));
					log_info(logKernel,"Los registros son : A es %d B es %d C es %d D es %d E es %d",atoi(array[3]),atoi(array[4]),atoi(array[5]),atoi(array[6]),atoi(array[7]));

				}

				if(header_recibido_de_la_CPU == CPU_TO_KERNEL_END_PROC){


					log_info(logKernel,"recibi el tcb porque finalizo proceso por XXXX");


				}

				return EXIT_SUCCESS;
		} // END for(;;)--and you thought it would never end!
		return EXIT_SUCCESS;
}
