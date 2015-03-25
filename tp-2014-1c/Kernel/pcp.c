/*
 * Pcp.c
 *
 *  Created on: 07/05/2014
 *      Author: utnso
 */

#include "kernel.h"
t_log* kernelLog;

void* pcp(t_plpThread *plpThread){

	int myPID = process_get_thread_id();
	log_info(kernelLog, "************** PCP Thread Started!(PID: %d) ***************",myPID);

		//Logica principal para administrar conexiones
		fd_set master; //file descriptor list
		fd_set read_fds; //file descriptor list temporal para el select()
		int fdmax; //maximo numero de file descriptor para hacer las busquedas en comunicaciones

		int listener;//socket escucha
		int newfd;//file descriptor del cliente aceptado
		struct sockaddr_storage remoteaddr; //dirección del cliente
		socklen_t addrlen;

		char remoteIP[INET6_ADDRSTRLEN];

		int i;

		struct addrinfo hints;

		FD_ZERO(&master);	//clear the master
		FD_ZERO(&read_fds); //clear the temp set

		 // get us a socket and bind it
		 //Lleno la estructura de tipo addrinfo
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;

		t_socket_info socketInfo;
		socketInfo.sin_addr.s_addr = INADDR_ANY;
		socketInfo.sin_family = AF_INET;
		socketInfo.sin_port = htons(config_kernel.PUERTO_CPU);
		memset(&(socketInfo.sin_zero), '\0', 8);

		listener = crearSocket();

		bindearSocket(listener, socketInfo);

		// listen turns on server mode for a socket.
		if (listen(listener, 10) == -1) {
			perror("listen");
			exit(3);
		}

		// add the listener to the master set
		FD_SET(listener, &master);

		// keep track of the biggest file descriptor
		fdmax = listener; // so far, it's this one

		// main loop
		for(;;) {

			read_fds = master; // copy it

			if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
				perror("select");
				exit(4);
			}

			// run through the existing connections looking for data to read
			for(i = 0; i <= fdmax; i++) {

				if (FD_ISSET(i, &read_fds)) { // we got one!!

					if (i == listener) {

						// handle new connections
						addrlen = sizeof remoteaddr;
						newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

						if (newfd == -1) {
							log_error(kernelLog, string_from_format("Hubo un error en el accept para el fd: %i", i));
						}
						else {
							FD_SET(newfd, &master); // add to master set
							if (newfd > fdmax) {    // keep track of the max
								fdmax = newfd;
							}

							//Shows the new connection administrated
							log_info(kernelLog, string_from_format("selectserver: new connection from %s on socket %d\n",
									inet_ntop(remoteaddr.ss_family,
									get_in_addr((struct sockaddr*)&remoteaddr),
									remoteIP, INET6_ADDRSTRLEN),
								newfd));
						}
					}
					else {

						/*handle data from a client*/
						t_contenido mensaje; //buffer para el dato del cliente
						memset(mensaje, 0, sizeof(t_contenido));
						t_header header = recibirMensaje(i, mensaje, kernelLog);

						switch(header){
						case ERR_CONEXION_CERRADA:


							//Removes from master set and say good bye! :)
							close(i); // bye!

							FD_CLR(i, &master); // remove from master set

							bool _match_cpu_fd(void* element){
								if(((t_client_cpu*)element)->cpuFD == i){
									return true;
								}
								return false;
							}

							pthread_mutex_lock(&mutex_cpu_list);
								t_client_cpu* cpu = list_find(cpu_client_list, (void*)_match_cpu_fd);
								int32_t processPID = cpu->isBusy ? cpu->processPID : 0;
								int32_t cpuPID = cpu->cpuPID;
								list_remove_and_destroy_by_condition(cpu_client_list, (void*)_match_cpu_fd, (void*)free);
							pthread_mutex_unlock(&mutex_cpu_list);

							log_info(kernelLog, string_from_format("PCP Thread says: Alguien mató al CPU (PID:%d)", cpuPID));

							if(processPID != 0){
								removeProcess(processPID, false);
								PrintQueues();
							}

							break;

						case(CPU_TO_KRN_HANDSHAKE):{


							//Creo la estructura del nuevo CPU con todos sus datos
							t_client_cpu* aCPU = malloc(sizeof(t_client_cpu));
							aCPU->cpuFD = i;
							aCPU->cpuPID = atoi(mensaje);
							aCPU->processFd = 0;
							aCPU->processPID = 0;
							aCPU->isBusy = false;

							log_info(kernelLog, string_from_format("PCP Thread says: Nuevo CPU conectado (PID: %d) aguarda instrucciones", aCPU->cpuPID));
							//Agrego el nuevo CPU a la lista.
							pthread_mutex_lock(&mutex_cpu_list);
								list_add(cpu_client_list, aCPU);
							pthread_mutex_unlock(&mutex_cpu_list);

							addExecProcess();
						}
						break;
					case (CPU_TO_KRN_END_PROC_QUANTUM_SIGNAL):{
						char** split = string_get_string_as_array(mensaje);
						int32_t pID_process = atoi(split[0]);
						int32_t programCounter = atoi(split[1]);
						int32_t contextoActual_size = atoi(split[2]);
						int32_t cursorCAct = atoi(split[3]);

						bool wasExecuting = false;

						log_info(kernelLog, string_from_format("CPU PID:%d, informa que termino de ejecutar el quantum del proceso PID:%d", ((t_client_cpu*)GetCPUByCPUFd(i))->cpuPID, pID_process));

						bool _match_pid(void* element){
							if(((t_process*)element)->process_pcb->pId == pID_process){
								return true;
							}
							return false;
						}

						//Busco el PCB que está en ejecución y lo encolo en lista de READY
						pthread_mutex_lock(&mutex_exec_queue);
							if(list_any_satisfy(exec_queue->elements, (void*)_match_pid)){
								wasExecuting = true;

								t_process* aProcess = list_find(exec_queue->elements, (void*)_match_pid);
									list_remove_by_condition(exec_queue->elements, (void*)_match_pid);
								//Actualizo los valores conforme a lo que me envió el CPU
								aProcess->process_pcb->programCounter = programCounter;
								aProcess->process_pcb->contextoActual_size = contextoActual_size;
								aProcess->process_pcb->cursorStack = cursorCAct;

								addReadyProcess(aProcess);
							}
						pthread_mutex_unlock(&mutex_exec_queue);


						//Seteo como disponible al procesador si y solo si, ya no le di otro programa para ejecutar!
						//Esto lo puede hacer uno de esos hilos de caza :P
						pthread_mutex_lock(&mutex_cpu_list);
							t_client_cpu* aCPU = GetCPUByCPUFd(i);
							aCPU->isBusy = true;
							aCPU->processFd = 0;
							aCPU->processPID = 0;
						pthread_mutex_unlock(&mutex_cpu_list);

						addExecProcess();
					}
					break;
					case(CPU_TO_KRN_END_PROC_QUANTUM):{

							char** split = string_get_string_as_array(mensaje);
							int32_t pID_process = atoi(split[0]);
							int32_t programCounter = atoi(split[1]);
							int32_t contextoActual_size = atoi(split[2]);
							int32_t cursorCAct = atoi(split[3]);
							
							bool wasExecuting = false;

							log_info(kernelLog, string_from_format("CPU PID:%d, informa que termino de ejecutar el quantum del proceso PID:%d", ((t_client_cpu*)GetCPUByCPUFd(i))->cpuPID, pID_process));

							bool _match_pid(void* element){
								if(((t_process*)element)->process_pcb->pId == pID_process){
									return true;
								}
								return false;
							}

							//Busco el PCB que está en ejecución y lo encolo en lista de READY
							pthread_mutex_lock(&mutex_exec_queue);
								if(list_any_satisfy(exec_queue->elements, (void*)_match_pid)){
									wasExecuting = true;

									t_process* aProcess = list_find(exec_queue->elements, (void*)_match_pid);
										list_remove_by_condition(exec_queue->elements, (void*)_match_pid);
									//Actualizo los valores conforme a lo que me envió el CPU
									aProcess->process_pcb->programCounter = programCounter;
									aProcess->process_pcb->contextoActual_size = contextoActual_size;
									aProcess->process_pcb->cursorStack = cursorCAct;

									addReadyProcess(aProcess);
								}
							pthread_mutex_unlock(&mutex_exec_queue);


							//Seteo como disponible al procesador si y solo si, ya no le di otro programa para ejecutar!
							//Esto lo puede hacer uno de esos hilos de caza :P
							pthread_mutex_lock(&mutex_cpu_list);
								t_client_cpu* aCPU = GetCPUByCPUFd(i);
								if((aCPU->isBusy && aCPU->processPID == pID_process) || wasExecuting){
									aCPU->isBusy = false;
									aCPU->processFd = 0;
									aCPU->processPID = 0;
									log_info(kernelLog, string_from_format("PCP Thread says: Cpu libre, espera instrucciones (PID: %d) aguarda instrucciones", aCPU->cpuPID));
								}
							pthread_mutex_unlock(&mutex_cpu_list);

							addExecProcess();
						}
						break;

					case(SYSCALL_IO_REQUEST):{

						int32_t fd;
						char** split = string_get_string_as_array(mensaje);

						int32_t pID_process = atoi(split[0]);
						t_nombre_dispositivo dispositivo = split[1];
						int32_t tiempo = atoi(split[2]);
						int32_t programCounter = atoi(split[3]);
						int32_t contextoActual_size = atoi(split[4]);
						int32_t cursorCAct = atoi(split[5]);

						bool _match_pid(void* element){
							if(((t_process*)element)->process_pcb->pId == pID_process){
								return true;
							}
							return false;
						}

						pthread_mutex_lock(&mutex_cpu_list);
							t_client_cpu* aCPU = GetCPUByCPUFd(i);
							aCPU->isBusy = false;
						pthread_mutex_unlock(&mutex_cpu_list);

						if(list_any_satisfy(exec_queue->elements, (void*)_match_pid)){
							pthread_mutex_lock(&mutex_exec_queue);
								t_process* aProcess = list_find(exec_queue->elements, (void*)_match_pid);

								aProcess->process_pcb->programCounter = programCounter + 1;
								aProcess->process_pcb->contextoActual_size = contextoActual_size;
								aProcess->process_pcb->cursorStack = cursorCAct;
								//TODO deberiamos verificar que exista el semaforo?
								strcpy( aProcess->blockedByIO, dispositivo);
								aProcess->io_tiempo = tiempo;
								fd = aProcess->process_fd;
							pthread_mutex_unlock(&mutex_exec_queue);

							addBlockedProcess(fd, NO_SEMAPHORE, dispositivo, tiempo);
						}
					}
						break;

					case(CPU_TO_KRN_END_PROC):{

							char** split = string_get_string_as_array(mensaje);

							int32_t pID_process = atoi(split[0]);

							bool _match_pid(void* element){
								if(((t_process*)element)->process_pcb->pId == pID_process){
									return true;
								}
								return false;
							}

							pthread_mutex_lock(&mutex_exec_queue);
								t_process* aProcess = list_find(exec_queue->elements, (void*)_match_pid);
								list_remove_by_condition(exec_queue->elements, (void*)_match_pid);
							pthread_mutex_unlock(&mutex_exec_queue);
							t_contenido mensaje;

							if((split[1] != NULL) && !(strlen(split[1]))==0){
								memset(mensaje, 0, sizeof(t_contenido));
								strcpy(mensaje, split[1]);
								enviarMensaje(aProcess->process_fd, KRN_TO_PRG_IMPR_VARIABLES, mensaje, kernelLog);
							}

							addExitProcess(aProcess);

							pthread_mutex_lock(&mutex_cpu_list);

								t_client_cpu* aCPU = GetCPUByCPUFd(i);
								aCPU->isBusy = false;
								aCPU->processFd = 0;
								aCPU->processPID = 0;

							pthread_mutex_unlock(&mutex_cpu_list);

							addExecProcess(aCPU);

							PrintQueues();
						}
						break;

					case SYSCALL_GET_REQUEST:{

						//TODO Checkear esta asignacion
						//t_nombre_variable var_buscada = mensaje;

						bool _match_var(void* element){
							if(string_equals_ignore_case(((t_var_compartida*)element)->Id, mensaje)){
								return true;
							}
							return false;
						}

						t_var_compartida* var_b = list_find(var_compartida_list, (void*)_match_var);

						if(var_b != NULL){

							memset(mensaje, 0, sizeof(t_contenido));
							strcpy(mensaje, string_from_format("%d", var_b->Valor));
							//t_contenido mensaje = var_b->Valor;
							enviarMensaje(i, KRN_TO_CPU_VAR_COMPARTIDA_OK, mensaje, kernelLog);
							log_info(kernelLog, string_from_format("Se envía a CPU el valor de la variable compartida requerida:%d", mensaje));

							t_header header = recibirMensaje(i, mensaje, kernelLog);
							if(header != CPU_TO_KRN_OK){
								log_info(kernelLog, "Error al recibir mensaje de CPU");
							}
							else{
								log_info(kernelLog,"Mensaje recibido correctamente");
							}
						}
						else{
							//t_contenido mensaje; la variable ya existe!
							memset(mensaje, 0, sizeof(t_contenido));
							if(enviarMensaje(i, KRN_TO_CPU_VAR_COMPARTIDA_ERROR, mensaje, kernelLog) != EXIT_FAILURE)
								log_info(kernelLog, "Se envia mensaje a CPU por variable no encontrada");

							t_header header = recibirMensaje(i, mensaje, kernelLog);
							if(header != CPU_TO_KRN_OK){
								log_info(kernelLog, "Error al recibir mensaje de CPU");
							}
							else{
								log_info(kernelLog,"Mensaje recibido correctamente");
							}
						}
						PrintQueues();
					}
						break;

					case SYSCALL_SET_REQUEST:{

						char** split = string_get_string_as_array(mensaje);
						t_nombre_compartida var_buscada = split[0];
						t_valor_variable valor = atoi(split[1]);

						bool _match_var(void* element){
							if(string_equals_ignore_case(((t_var_compartida*)element)->Id, var_buscada)){
								return true;
							}
							return false;
						}

						if(list_any_satisfy(var_compartida_list, (void*)_match_var)){

							t_var_compartida* var_b = list_find(var_compartida_list, (void*)_match_var);
							var_b->Valor = valor;//ver que pasa cuando modifico

							//t_contenido mensaje = var_b->Valor; la variable ya existe!
							memset(mensaje, 0, sizeof(t_contenido));
							strcpy(mensaje, string_from_format("%d", var_b->Valor));

							enviarMensaje(i, KRN_TO_CPU_ASIGNAR_OK, mensaje, kernelLog);
							log_info(kernelLog, "Se envia respuesta afirmativa a CPU");
						}
						else{
							enviarMensaje(i, KRN_TO_CPU_VAR_COMPARTIDA_ERROR, mensaje, kernelLog);
							log_info(kernelLog, "Se envia respuesta afirmativa a CPU");
						}
					}
						break;

					case CPU_TO_KRN_IMPRIMIR:{

						bool _match_cpu_fd(void* element){
							if(((t_client_cpu*)element)->cpuFD == i){
								return true;
							}
							return false;
						}

						pthread_mutex_lock(&mutex_cpu_list);
						t_client_cpu* cpu = list_find(cpu_client_list, (void*)_match_cpu_fd);
						pthread_mutex_unlock(&mutex_cpu_list);

						enviarMensaje(cpu->processFd, KRN_TO_PRG_IMPR_PANTALLA, mensaje, kernelLog);

						log_info(kernelLog, string_from_format("Se envia mensaje a programa en ejecucion para que imprima este valor:%s", mensaje));
					}
						break;

					case CPU_TO_KRN_IMPRIMIR_TEXTO:{

						bool _match_cpu_fd(void* element){
							if(((t_client_cpu*)element)->cpuFD == i){
								return true;
							}
							return false;
						}

						pthread_mutex_lock(&mutex_cpu_list);
						t_client_cpu* cpu = list_find(cpu_client_list, (void*)_match_cpu_fd);
						pthread_mutex_unlock(&mutex_cpu_list);

						enviarMensaje(cpu->processFd, KRN_TO_PRG_IMPR_IMPRESORA, mensaje, kernelLog);

						log_info(kernelLog, "Se envia cadena de texto para que lo imprima el programa");

					}
						break;

					case SYSCALL_WAIT_REQUEST:
					{
						int32_t fd;
						char** split = string_get_string_as_array(mensaje);

						t_nombre_semaforo semaforo = split[0];
						int32_t pID_actual = atoi(split[1]);
						int32_t programCounter = atoi(split[2]);
						int32_t contextoActual_size = atoi(split[3]);
						int32_t cursorCAct = atoi(split[4]);

						log_info(kernelLog, string_from_format("El CPU me solicita usar el semaforo: %s", semaforo));

						//busco semaforo ansisop en la lista y hago lo correspondiente
						bool _match_sem(void* element){
							if(string_equals_ignore_case(((t_semaforoAnsisop*)element)->Id, semaforo)){
								return true;
							}
							return false;
						}

						bool _match_pid(void* element){
							if(((t_process*)element)->process_pcb->pId == pID_actual){
								return true;
							}
							return false;
						}

						t_semaforoAnsisop* semaforoActual = list_find(semaforosAnsisop_list, (void*)_match_sem);
						if(semaforoActual != NULL)
						{
							log_info(kernelLog, "Se encontro el semaforo requerido!...");
							log_info(kernelLog, "****Semaforo ID: %s", semaforoActual->Id);
							log_info(kernelLog, "****Semaforo-Antiguo Valor: %d", semaforoActual->Valor);

							semaforoActual->Valor --;

							log_info(kernelLog, "****Semaforo-Nuevo Valor: %d", semaforoActual->Valor);

							if(semaforoActual->Valor >= 0){

								memset(mensaje,0,sizeof(t_contenido));
								enviarMensaje(i, KRN_TO_CPU_OK, mensaje, kernelLog);
								log_info(kernelLog, "Se envio al CPU la respuesta de la funcion wait aplicada al semaforo requerido");

								t_header header = recibirMensaje(i, mensaje, kernelLog);
								if(header != CPU_TO_KRN_OK){
									log_info(kernelLog, "Error al recibir mensaje de CPU");
								}
								else{
									log_info(kernelLog,"El CPU recibio el mensaje correctamente");
								}
							}
							else{

								memset(mensaje,0,sizeof(t_contenido));
								enviarMensaje(i, KRN_TO_CPU_BLOCKED, mensaje, kernelLog);
								log_info(kernelLog, "Se envia al CPU la respuesta de la funcion wait aplicada al semaforo requerido");

								t_header header = recibirMensaje(i, mensaje, kernelLog);

								if(header != CPU_TO_KRN_OK){

									log_error(kernelLog, "Error al recibir mensaje de CPU");
								}
								else{

									log_info(kernelLog,"Mensaje recibido correctamente de CPU");

									pthread_mutex_lock(&mutex_exec_queue);
										t_process* aProcess = list_find(exec_queue->elements, (void*)_match_pid);
										aProcess->process_pcb->programCounter = programCounter + 1;
										aProcess->process_pcb->contextoActual_size = contextoActual_size;
										aProcess->process_pcb->cursorStack = cursorCAct;
										fd = aProcess->process_fd;
									pthread_mutex_unlock(&mutex_exec_queue);

									addBlockedProcess(fd, semaforoActual->Id, NO_IO, 0);
									
									pthread_mutex_lock(&mutex_cpu_list);

										t_client_cpu* aCPU = GetCPUByCPUFd(i);
										aCPU->isBusy = false;
										aCPU->processFd = 0;
										aCPU->processPID = 0;
									pthread_mutex_unlock(&mutex_cpu_list);

									addExecProcess();

								}
							}
						}
						else{
							log_error(kernelLog, "NO SE ENCONTRO EL SEMAFORO REQUERIDO (ID: %s)", semaforo);
						}


					}
					break;

					case SYSCALL_SIGNAL_REQUEST:
					{

						t_process* aProcess;

						bool _match_proc_sem(void* element){
							if(string_equals_ignore_case(((t_process*)element)->blockedBySemaphore, mensaje)){
								return true;
							}
							return false;
						}

						bool _match_sem(void* element){
							if(string_equals_ignore_case(((t_semaforoAnsisop*)element)->Id, mensaje)){
								return true;
							}
							return false;
						}



						//busco semaforo ansisop en la lista y hago lo correspondiente
						t_semaforoAnsisop* semaforoActual = list_find(semaforosAnsisop_list, (void*)_match_sem);
						semaforoActual->Valor++;

						if(semaforoActual->Valor <= 0){

							pthread_mutex_lock(&mutex_block_queue);

								if(list_any_satisfy(block_queue->elements, (void*)_match_proc_sem)){

									//Creo una lista auxiliar de los procesos que estan bloqueados por este semaforo
									t_list* auxList = list_filter(block_queue->elements, (void*)_match_proc_sem);
									aProcess = (t_process*)list_remove(auxList, 0);

											bool _match_pid(void* element){
												if(((t_process*)element)->process_pcb->pId == aProcess->process_pcb->pId){
													return true;
												}
												return false;
											}

									//Remuevo el proceso que esta bloqueado por este semaforo de la lista de BLOCKED
									list_remove_by_condition(block_queue->elements, (void*)_match_pid);

									addReadyProcess(aProcess);
									log_debug(kernelLog, "Agrege uno en READY por SIGNAL");
									addExecProcess();

									//Libero memoria
									free(auxList);
								}
								else{
									log_info(kernelLog, string_from_format("Hubo un proceso bloqueado por el semaforo %s, pero actualmente no esta en el sistema. Pudo haber sido eliminado!", mensaje));
								}

							pthread_mutex_unlock(&mutex_block_queue);
						}

					}
					break;

					case CPU_TO_KRN_END_PROC_ERROR:
					{
						bool anyone = false;
						t_process* aProcess;

						bool _match_pid(void* element){
							if(((t_process*)element)->process_pcb->pId == atoi(mensaje)){
								return true;
							}
							return false;
						}

						//valido que exista en el programa en ejecucion (lo pueden haber matado)
						pthread_mutex_lock(&mutex_exec_queue);
							if(list_any_satisfy(exec_queue->elements, (void*)_match_pid)){
								aProcess = list_find(exec_queue->elements, (void*)_match_pid);
								list_remove_by_condition(exec_queue->elements, (void*)_match_pid);
								anyone = true;
							}
						pthread_mutex_unlock(&mutex_exec_queue);

						if(anyone){/* Bye! :) */
							addExitProcess(aProcess);
						}

						pthread_mutex_lock(&mutex_cpu_list);

							t_client_cpu* aCPU = GetCPUByCPUFd(i);
							aCPU->isBusy = false;
							aCPU->processFd = 0;
							aCPU->processPID = 0;

						pthread_mutex_unlock(&mutex_cpu_list);

						addExecProcess();

					}
					break;

					default:
						;
					}
					} // END handle data from client
				} // END got new incoming connection
			} // END looping through file descriptors
		} // END for(;;)--and you thought it would never end!
		return EXIT_SUCCESS;
}
