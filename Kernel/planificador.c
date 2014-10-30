#include "kernel.h"



void* planificador(){
	int myPID = process_get_thread_id();
		log_info(logKernel, "************** PLANIFICADOR!(PID: %d) ***************",myPID);

		fd_set master; //file descriptor list
			fd_set read_fds; //file descriptor list temporal para el select()
			int fdmax; //maximo numero de file descriptor para hacer las busquedas en comunicaciones

			int listener; //socket escucha
			int newfd; //file descriptor del cliente aceptado
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

		for(;;){

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
							log_error(logKernel, string_from_format("Hubo un error en el accept para el fd: %i", i));
						} else {

							FD_SET(newfd, &master); // add to master set
							if (newfd > fdmax) {    // keep track of the max
								fdmax = newfd;
							}

							//Shows the new connection administrated
							log_info(logKernel, string_from_format("selectserver: new connection from %s on socket %d\n",
									inet_ntop(remoteaddr.ss_family,
									get_in_addr((struct sockaddr*)&remoteaddr),
									remoteIP, INET6_ADDRSTRLEN), newfd));
						}
					} else {

						/*handle data from a client*/
						t_contenido mensaje; //buffer para el dato del cliente
						memset(mensaje, 0, sizeof(t_contenido));
						t_header header = recibirMensaje(i, mensaje, logKernel);

						switch (header) {
						case ERR_CONEXION_CERRADA:{

						}
							break;
						case ERR_ERROR_AL_RECIBIR_MSG:{

						}
							break;
						case CPU_TO_KERNEL_HANDSHAKE:{
														//Creo la estructura del nuevo CPU con todos sus datos
														log_info(logKernel, ("hola se conecto algo"));
														t_client_cpu* aCPU = malloc(sizeof(t_client_cpu));
														aCPU->cpuFD = i;
														aCPU->cpuPID = atoi(mensaje);
														aCPU->processFd = 0;
														aCPU->processPID = 0;
														aCPU->ocupado = false;

														log_info(logKernel, string_from_format("Planificador dice: Nuevo CPU conectado (PID: %d) aguarda instrucciones", aCPU->cpuPID));
														//Agrego el nuevo CPU a la lista.
														pthread_mutex_lock(&mutex_cpu_list);
															list_add(cpu_disponibles_list, aCPU);
															log_info(logKernel, string_from_format("Agregue una cpu nueva a la lista para (PID: %d) aguarda instrucciones", aCPU->cpuPID));
														pthread_mutex_unlock(&mutex_cpu_list);
						}
							break;
						case CPU_TO_KERNEL_FINALIZO_QUANTUM_NORMALMENTE: {// EXEC -> READY

							char** split = string_get_string_as_array(mensaje);
														int32_t pID_actual = atoi(split[0]);
														int32_t programCounter = atoi(split[1]);
														int32_t contextoActual_size = atoi(split[2]);
														int32_t cursorStackAct = atoi(split[3]);

														bool wasExecuting = false;

														log_info(logKernel, string_from_format("CPU PID:%d, informa que termino de ejecutar el quantum del proceso PID:%d", ((t_client_cpu*)encontrarCPUporFd(i))->cpuPID, pID_actual));

														bool _match_pid(void* element){
															if(((t_process*)element)->tcb->pid == pID_actual){
																return true;
															}
															return false;
														}

														//Llevo el tcb de ejecución y aREADY
														pthread_mutex_lock(&mutex_exec_queue);
															if(list_any_satisfy(EXEC->elements, (void*)_match_pid)){
																wasExecuting = true;

																t_process* aProcess = list_find(EXEC->elements, (void*)_match_pid);
																list_remove_by_condition(EXEC->elements, (void*)_match_pid);
																//CPU me envia los nuevos valores
																aProcess->tcb->program_counter = programCounter;
																aProcess->tcb->tamanio_indice_codigo = contextoActual_size;
																aProcess->tcb->cursor_stack = cursorStackAct;

																agregarProcesoColaReady(aProcess);
															}
														pthread_mutex_unlock(&mutex_exec_queue);


														//Ahora compruebo si el procesaador queda como disponible
														pthread_mutex_lock(&mutex_cpu_list);
															t_client_cpu* aCPU = encontrarCPUporFd(i);
															if((aCPU->ocupado && aCPU->processPID == pID_actual) || wasExecuting){
																aCPU->ocupado = false;
																aCPU->processFd = 0;
																aCPU->processPID = 0;
																log_info(logKernel, string_from_format("Planificador: La cpu (PID: %d) esta libre y puede utilizarse", aCPU->cpuPID));
															}
														pthread_mutex_unlock(&mutex_cpu_list);

														agregarProcesoColaExec(); //YA QUE HAY UNA CPU DISPONIBLE !!
						}
							break;
						case CPU_TO_KERNEL_END_PROC: // EXEC -> EXIT. CPU ME MANDA EL PID
						{
							char** split = string_get_string_as_array(mensaje);

														int32_t pID_actual = atoi(split[0]);

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

															t_client_cpu* aCPU = encontrarCPUporFd(i);
															aCPU->ocupado = false;
															aCPU->processFd = 0;
															aCPU->processPID = 0;

														pthread_mutex_unlock(&mutex_cpu_list);

														agregarProcesoColaExec();

														mostrarColas();

						}

							break;
						case CPU_TO_KERNEL_END_PROC_ERROR:{ // EXEC -> EXIT
							bool existe = false;
													t_process* aProcess;

													bool _match_pid(void* element){
														if(((t_process*)element)->tcb->pid== atoi(mensaje)){
															return true;
														}
														return false;
													}

													//validor que exista en ejecucion
													pthread_mutex_lock(&mutex_exec_queue);
														if(list_any_satisfy(EXEC->elements, (void*)_match_pid)){
															aProcess = list_find(EXEC->elements, (void*)_match_pid);
															list_remove_by_condition(EXEC->elements, (void*)_match_pid);
															existe = true;
														}
													pthread_mutex_unlock(&mutex_exec_queue);

													if(existe){
														agregarProcesoColaExit(aProcess);
													}

													pthread_mutex_lock(&mutex_cpu_list);

														t_client_cpu* aCPU = encontrarCPUporFd(i);
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
					}
			}
		return NULL;
		}
		}
}




