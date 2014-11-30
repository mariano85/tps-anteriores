/*
 * planificador.c
 *
 *  Created on: 17/10/2014
 *      Author: utnso
 */



#include "kernel.h"
t_log* kernelLog;

void* planificador_nico(t_thread *loaderThread){


	int32_t pid = 1234;
//	int32_t base_del_segmento_codigo = 1048576;
	int32_t program_counter = 1048576;
	int32_t base_del_stack = 0;
	int32_t cursor_de_stack = 0;
	int32_t QUANTUM = 14;
	int32_t MODO = 1;



	// Le mando el TCB a CPU

	t_contenido mensaje_1;
	memset(mensaje_1, 0, sizeof(t_contenido));
	strcpy(mensaje_1, string_from_format("[%d,%d,%d,%d,%d,%d]", pid,program_counter,QUANTUM,MODO,base_del_stack,cursor_de_stack));
	enviarMensaje(socket_cpu, KERNEL_TO_CPU_TCB, mensaje_1, logKernel);
	log_info(logKernel, "Se envía un TCB al CPU libre elegido");

		for(;;) {


				log_info(logKernel,"Me meti en el planificador");


				//Espero a que cpu me devuelva el TCB

				t_contenido mensaje_de_la_cpu;
				memset(mensaje_de_la_cpu,0,sizeof(t_contenido));
				t_header header_recibido_de_la_CPU = recibirMensaje(socket_cpu,mensaje_de_la_cpu,logKernel);

				//1# Si entra aca es porque al proceso se le acabo el QUANTUM hay que mandarlo a la cola de ready

				if(header_recibido_de_la_CPU == CPU_TO_KERNEL_FINALIZO_QUANTUM_NORMALMENTE){

					char** array = string_get_string_as_array(mensaje_de_la_cpu);

					//Aca hay que guardarlo en un estructura TCB y mandarlo a la cola de ready pero no se cual estructura usan te lo dejo a vos lean
					//MUESTRO LO QUE DEBERIA LLEGARTE IGUAL

					log_info(logKernel,"PID ES %d, Program_counter es %d ,modo es %d",atoi(array[0]),atoi(array[1]),atoi(array[2]));
					log_info(logKernel,"Los registros son : A es %d B es %d C es %d D es %d E es %d",atoi(array[3]),atoi(array[4]),atoi(array[5]),atoi(array[6]),atoi(array[7]));

				}

				//2# Si entra es porque el proceso finalizo hay que mandarlo a la cola de exit

				if(header_recibido_de_la_CPU == CPU_TO_KERNEL_END_PROC){


					log_info(logKernel,"recibi el tcb porque finalizo proceso por XXXX");


				}

				//3# Ingresa un valor que se lo manda a la cpu para que lo almacene en el registro A

				if(header_recibido_de_la_CPU == CPU_TO_KERNEL_BLOQUEO_INNN){

					log_info(logKernel,"ENTRE AL IF CON INNN");

					int32_t valor;

					printf("Ingrese un valor");
					scanf("%d",&valor);

					t_contenido mensaje_para_cpu;
					memset(mensaje_para_cpu,0,sizeof(t_contenido));
					strcpy(mensaje_para_cpu, string_from_format("[%d]",valor));
					enviarMensaje(socket_cpu,KERNEL_TO_CPU_OK,mensaje_para_cpu,logKernel);


				}

				//4# Ingresa una cadena de caracteres por consola y lo manda a la cpu para guardarlo en memoria

				if(header_recibido_de_la_CPU == CPU_TO_KERNEL_BLOQUEO_INNC){

					log_info(logKernel,"ENTRE AL IF CON INNC");
					char** array = string_get_string_as_array(mensaje_de_la_cpu);

					int32_t B = atoi(array[0]);

					char* cadena = malloc(B);


					printf("Ingrese una cadena de caracteres menor a %d",B);
					scanf("%s",cadena);

					log_info(logKernel,"el valor de la cadena es %s",cadena);

					t_contenido mensaje_para_cpu;
					memset(mensaje_para_cpu,0,B);
					memcpy(mensaje_para_cpu,cadena,B);
					enviarMensaje(socket_cpu,KERNEL_TO_CPU_OK,mensaje_para_cpu,logKernel);


				}

				if(header_recibido_de_la_CPU == CPU_TO_MSP_SOLICITAR_BYTES_REGISTRO){


					return EXIT_SUCCESS;


				}

		} // END for(;;)--and you thought it would never end!
		return EXIT_SUCCESS;
}
