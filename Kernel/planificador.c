/*
 * planificador.c
 *
 *  Created on: 17/10/2014
 *      Author: utnso
 */



#include "kernel.h"

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


				if(header_recibido_de_la_CPU == CPU_TO_KERNEL_HANDSHAKE){

									char** array = string_get_string_as_array(mensaje_de_la_cpu);

									//Creo la estructura del nuevo CPU con todos sus datos
									t_client_cpu* aCPU = malloc(sizeof(t_client_cpu));
									aCPU->cpuFD = socket_cpu;
									aCPU->cpuPID = atoi(array[1]);
									aCPU->processFd = atoi(array[2]);
									aCPU->processPID = 0;
									aCPU->ocupado = false;

									log_info(logKernel, string_from_format("Planificador: Nuevo CPU conectado (PID: %d) aguarda instrucciones", aCPU->cpuPID));
									//Agrego el nuevo CPU a la lista.
									pthread_mutex_lock(&mutex_cpu_list);
									list_add(cpu_disponibles_list, aCPU);
									log_info(logKernel, string_from_format("agregue cpu (PID: %d) aguarda instrucciones", aCPU->cpuPID));
									pthread_mutex_unlock(&mutex_cpu_list);

									agregarProcesoColaExec();
								}





				if(header_recibido_de_la_CPU == CPU_TO_KERNEL_FINALIZO_QUANTUM_NORMALMENTE){

					char** array = string_get_string_as_array(mensaje_de_la_cpu);

					//Aca hay que guardarlo en un estructura TCB y mandarlo a la cola de ready pero no se cual estructura usan te lo dejo a vos lean
					//MUESTRO LO QUE DEBERIA LLEGARTE IGUAL

					int pID_process = atoi(array[0]);
					int programCounter = atoi(array[1]);
					int modoKernel = atoi(array[2]);
					int An = atoi(array[3]);
					int Bn = atoi(array[4]);
					int Cn = atoi(array[5]);
					int Dn = atoi(array[6]);
					int En = atoi(array[7]);

					bool wasExecuting = false;

					log_info(logKernel, string_from_format("CPU informa que termino de ejecutar el quantum del proceso PID:%d",  pID_process));

					bool _match_pid(void* element){
					if(((t_process*)element)->tcb->pid == pID_process){
					return true;
					}
					return false;
					}

					//Busco el PCB que está en ejecución y lo encolo en lista de READY
					pthread_mutex_lock(&mutex_exec_queue);
					if(list_any_satisfy(EXEC->elements, (void*)_match_pid)){
					wasExecuting = true;

					t_process* aProcess = list_find(EXEC->elements, (void*)_match_pid);
					list_remove_by_condition(EXEC->elements, (void*)_match_pid);
														//Actualizo los valores conforme a lo que me envió el CPU
														aProcess->tcb->pid = pID_process;
														aProcess->tcb->program_counter = programCounter;
														aProcess->tcb->indicador_modo_kernel = modoKernel;
														aProcess->tcb->registros_de_programacion->A = An;
														aProcess->tcb->registros_de_programacion->B = Bn;
														aProcess->tcb->registros_de_programacion->C = Cn;
														aProcess->tcb->registros_de_programacion->D = Dn;
														aProcess->tcb->registros_de_programacion->E = En;


														agregarProcesoColaReady(aProcess);
													}
					pthread_mutex_unlock(&mutex_exec_queue);


												//Seteo como disponible al procesador si y solo si, ya no le di otro programa para ejecutar!
												//Esto lo puede hacer uno de esos hilos de caza :P
												pthread_mutex_lock(&mutex_cpu_list);
													t_client_cpu* aCPU = encontrarCPUporFd(socket_cpu);
													if((aCPU->ocupado && aCPU->processPID == pID_process) || wasExecuting){
														aCPU->ocupado = false;
														aCPU->processFd = 0;
														aCPU->processPID = 0;
														log_info(logKernel, string_from_format("Planificador: Cpu libre, espera instrucciones (PID: %d) aguarda instrucciones", aCPU->cpuPID));
													}
												pthread_mutex_unlock(&mutex_cpu_list);

												agregarProcesoColaExec();
											}







				if(header_recibido_de_la_CPU == CPU_TO_KERNEL_END_PROC){

					char** array = string_get_string_as_array(mensaje_de_la_cpu);

					int32_t pID_actual = atoi(array[0]);

					bool _match_pid(void* element){
					if(((t_process*)element)->tcb->pid == pID_actual){
					return true;
					}
					return false;
					}
					pthread_mutex_lock(&mutex_exec_queue);
					t_process* aProcess = list_find(EXEC->elements, (void*)_match_pid);
					list_remove_by_condition(EXEC->elements, (void*)_match_pid);
					pthread_mutex_unlock(&mutex_exec_queue);

					agregarProcesoColaExit(aProcess);
					//LIBERO UNA CPU!

					pthread_mutex_lock(&mutex_cpu_list);

					t_client_cpu* aCPU = encontrarCPUporFd(socket_cpu);
					aCPU->ocupado = false;
					aCPU->processFd = 0;
					aCPU->processPID = 0;

					pthread_mutex_unlock(&mutex_cpu_list);

					agregarProcesoColaExec();

					mostrarColas();



					log_info(logKernel,"recibi el tcb porque finalizo proceso por XXXX");


				}

				return EXIT_SUCCESS;
		} // END for(;;)--and you thought it would never end!
		return EXIT_SUCCESS;
}
