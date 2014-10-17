/*
 * planificador.c
 *
 *  Created on: 17/10/2014
 *      Author: utnso
 */



#include "kernel.h"
t_log* kernelLog;

void* planificador(t_loaderThread *loaderThread){

	int myPID = process_get_thread_id();
	log_info(kernelLog, "************** Comienza el planificador!(PID: %d) ***************",myPID);

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
								int32_t processPID = cpu->ocupado ? cpu->processPID : 0;
								int32_t cpuPID = cpu->cpuPID;
								list_remove_and_destroy_by_condition(cpu_client_list, (void*)_match_cpu_fd, (void*)free);
							pthread_mutex_unlock(&mutex_cpu_list);

							log_info(kernelLog, string_from_format("Planificador dice: Alguien mató al CPU (PID:%d)", cpuPID));

							if(processPID != 0){
								removeProcess(processPID, false);
								mostrarColas();
							}

							break;

						case(CPU_TO_KRN_HANDSHAKE):{


							//Creo la estructura del nuevo CPU con todos sus datos
							t_client_cpu* aCPU = malloc(sizeof(t_client_cpu));
							aCPU->cpuFD = i;
							aCPU->cpuPID = atoi(mensaje);
							aCPU->processFd = 0;
							aCPU->processPID = 0;
							aCPU->ocupado = false;

							log_info(kernelLog, string_from_format("PCP Thread says: Nuevo CPU conectado (PID: %d) aguarda instrucciones", aCPU->cpuPID));
							//Agrego el nuevo CPU a la lista.
							pthread_mutex_lock(&mutex_cpu_list);
								list_add(cpu_client_list, aCPU);
							pthread_mutex_unlock(&mutex_cpu_list);

							agregarProcesoColaExec();
						}
						break;
					case (CPU_TO_KRN_END_PROC_QUANTUM_SIGNAL):{
						char** split = string_get_string_as_array(mensaje);
						int32_t pID_process = atoi(split[0]);
						int32_t programCounter = atoi(split[1]);
						int32_t cursorCAct = atoi(split[3]);

						bool wasExecuting = false;

						log_info(kernelLog, string_from_format("CPU PID:%d, informa que termino de ejecutar el quantum del proceso PID:%d", ((t_client_cpu*)GetCPUByCPUFd(i))->cpuPID, pID_process));

						bool _match_pid(void* element){
							if(((t_process*)element)->tcb->pid == pID_process){
								return true;
							}
							return false;
						}

						//Busco el TCB que está en ejecución y lo encolo en lista de READY
						pthread_mutex_lock(&mutex_exec_queue);

						if(list_any_satisfy(EXEC->elements, (void*)_match_pid)){
								wasExecuting = true;

								t_process* aProcess = list_find(EXEC->elements, (void*)_match_pid);
								list_remove_by_condition(EXEC->elements, (void*)_match_pid);

							aProcess->tcb->program_counter = programCounter;

							aProcess->tcb->cursor_stack = cursorCAct;

							agregarProcesoColaReady(aProcess);
													}
						pthread_mutex_unlock(&mutex_exec_queue);


						//Seteo como disponible al procesador si y solo si, ya no le di otro programa para ejecutar!
						//Esto lo puede hacer uno de esos hilos de caza :P
						pthread_mutex_lock(&mutex_cpu_list);
							t_client_cpu* aCPU = GetCPUByCPUFd(i);
							aCPU->ocupado = true;
							aCPU->processFd = 0;
							aCPU->processPID = 0;
						pthread_mutex_unlock(&mutex_cpu_list);

						agregarProcesoColaExec();
					}
					break;
					case(CPU_TO_KRN_END_PROC_QUANTUM):{

							char** split = string_get_string_as_array(mensaje);
							int32_t pID_process = atoi(split[0]);
							int32_t programCounter = atoi(split[1]);
							int32_t cursorCAct = atoi(split[2]);





							bool wasExecuting = false;

							log_info(kernelLog, string_from_format("CPU PID:%d, informa que termino de ejecutar el quantum del proceso PID:%d", ((t_client_cpu*)GetCPUByCPUFd(i))->cpuPID, pID_process));

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
									aProcess->tcb->program_counter = programCounter;
									aProcess->tcb->cursor_stack = cursorCAct;

									agregarProcesoColaReady(aProcess);
								}
							pthread_mutex_unlock(&mutex_exec_queue);


							//Seteo como disponible al procesador si y solo si, ya no le di otro programa para ejecutar!
							//Esto lo puede hacer uno de esos hilos de caza :P
							pthread_mutex_lock(&mutex_cpu_list);
								t_client_cpu* aCPU = GetCPUByCPUFd(i);
								if((aCPU->ocupado && aCPU->processPID == pID_process) || wasExecuting){
									aCPU->ocupado = false;
									aCPU->processFd = 0;
									aCPU->processPID = 0;
									log_info(kernelLog, string_from_format("PCP Thread says: Cpu libre, espera instrucciones (PID: %d) aguarda instrucciones", aCPU->cpuPID));
								}
							pthread_mutex_unlock(&mutex_cpu_list);

							agregarProcesoColaExec();
						}
						break;



					case(CPU_TO_KRN_END_PROC):{

							char** split = string_get_string_as_array(mensaje);

							int32_t pID_process = atoi(split[0]);

							bool _match_pid(void* element){
								if(((t_process*)element)->tcb->pid == pID_process){
									return true;
								}
								return false;
							}

							pthread_mutex_lock(&mutex_exec_queue);
								t_process* aProcess = list_find(EXEC->elements, (void*)_match_pid);
								list_remove_by_condition(EXEC->elements, (void*)_match_pid);
							pthread_mutex_unlock(&mutex_exec_queue);
							t_contenido mensaje;

							if((split[1] != NULL) && !(strlen(split[1]))==0){
								memset(mensaje, 0, sizeof(t_contenido));
								strcpy(mensaje, split[1]);
								enviarMensaje(aProcess->process_fd, KRN_TO_PRG_IMPR_VARIABLES, mensaje, kernelLog);
							}

							agregarProcesoColaExit(aProcess);

							pthread_mutex_lock(&mutex_cpu_list);

								t_client_cpu* aCPU = GetCPUByCPUFd(i);
								aCPU->ocupado = false;
								aCPU->processFd = 0;
								aCPU->processPID = 0;

							pthread_mutex_unlock(&mutex_cpu_list);

							agregarProcesoColaExec();

							mostrarColas();
						}
						break;


					case CPU_TO_KRN_END_PROC_ERROR:
					{
						bool anyone = false;
						t_process* aProcess;

						bool _match_pid(void* element){
							if(((t_process*)element)->tcb->pid == atoi(mensaje)){
								return true;
							}
							return false;
						}

						//valido que exista en el programa en ejecucion (lo pueden haber matado)
						pthread_mutex_lock(&mutex_exec_queue);
							if(list_any_satisfy(EXEC->elements, (void*)_match_pid)){
								aProcess = list_find(EXEC->elements, (void*)_match_pid);
								list_remove_by_condition(EXEC->elements, (void*)_match_pid);
								anyone = true;
							}
						pthread_mutex_unlock(&mutex_exec_queue);

						if(anyone){/* Bye! :) */
							agregarProcesoColaExit(aProcess);
						}

						pthread_mutex_lock(&mutex_cpu_list);

							t_client_cpu* aCPU = GetCPUByCPUFd(i);
							aCPU->ocupado = false;
							aCPU->processFd = 0;
							aCPU->processPID = 0;

						pthread_mutex_unlock(&mutex_cpu_list);

						agregarProcesoColaExec();

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
