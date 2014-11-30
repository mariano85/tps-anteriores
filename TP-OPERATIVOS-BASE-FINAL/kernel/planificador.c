/*
 * planificador.c
 *
 *  Created on: 17/10/2014
 *      Author: utnso
 */



#include "kernel.h"
t_log* kernelLog;



void* planificador(t_loaderThread *planificadorThread){


	log_info(logKernel,"ENTRO AL PLANIFICADORRRRR");


//	int myPID = process_get_thread_id();
//		log_info(kernelLog, "************** Comienza el planificador!(PID: %d) ***************",myPID);

		//Logica principal para administrar conexiones
		fd_set master; //file descriptor list
		fd_set read_fds; //file descriptor list temporal para el select()
		int fdmax; //maximo numero de file descriptor para hacer las busquedas en comunicaciones

		int listener;//socket escucha
		int newfd;//file descriptor del cliente aceptado
		struct sockaddr_storage remoteaddr; //dirección del cliente
		socklen_t addrlen;

		char remoteIP[INET6_ADDRSTRLEN];

		int i; // socket entrante

		struct addrinfo hints;

		FD_ZERO(&master);	//clear the master
		FD_ZERO(&read_fds); //clear the temp set

		 //Lleno la estructura de tipo addrinfo
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;

		t_socket_info socketInfo;
		socketInfo.sin_addr.s_addr = INADDR_ANY;
		socketInfo.sin_family = AF_INET;
		socketInfo.sin_port = htons(7000);
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
						newfd = accept(listener, (struct sockaddr *) &remoteaddr,
								&addrlen);

						if (newfd == -1) {
							log_error(logKernel, string_from_format( "Hubo un error en el accept para el fd: %i", i));
						} else {
							FD_SET(newfd, &master); // add to master set
							if (newfd > fdmax) {    // keep track of the max
								fdmax = newfd;
							}

							//Shows the new connection administrated
							log_info(logKernel,
									string_from_format(
											"selectserver: new connection from %s on socket %d\n",
											inet_ntop(remoteaddr.ss_family,
													get_in_addr(
															(struct sockaddr*) &remoteaddr),
													remoteIP, INET6_ADDRSTRLEN),
											newfd));
						}

					} else {

						// TODO: loguear cada transicion de estados de los procesos!

										t_contenido mensaje_de_la_cpu; //buffer para el dato del cliente
										memset(mensaje_de_la_cpu, 0, sizeof(t_contenido));
										t_header header = recibirMensaje(i, mensaje_de_la_cpu, logKernel);
										char** array = string_get_string_as_array(mensaje_de_la_cpu);

										switch (header) {
										case ERR_CONEXION_CERRADA:

											// cerramos socket y lo borramos del master set
											close(i);
											FD_CLR(i, &master);

											// eliminamos la cpu de la lista de cpu's y el proceso que estaba ejecutando, si hubiere
											eliminarCpu(i);

											break;
										case ERR_ERROR_AL_RECIBIR_MSG:
											// TODO y aca que hacemos?
											break;

										case CPU_TO_KERNEL_HANDSHAKE :

											// Le mando el TCB a CPU

											// agrego cpu a la lista
											agregarCpu(i, mensaje_de_la_cpu);

											// muevo un proceso de READY -> EXEC y lo mando a ejecutar a la pc disponible (que deberia tener)
											log_info(logKernel, "Se envía un TCB al CPU libre elegido");
											agregarProcesoColaExec();

											break;
										case CPU_TO_KERNEL_FINALIZO_QUANTUM_NORMALMENTE:


											//Aca hay que guardarlo en un estructura TCB y mandarlo a la cola de ready pero no se cual estructura usan te lo dejo a vos lean
											//MUESTRO LO QUE DEBERIA LLEGARTE IGUAL

											log_info(logKernel,"PID ES %d, Program_counter es %d ,modo es %d",atoi(array[0]),atoi(array[1]),atoi(array[2]));
											log_info(logKernel,"Los registros son : A es %d B es %d C es %d D es %d E es %d",atoi(array[3]),atoi(array[4]),atoi(array[5]),atoi(array[6]),atoi(array[7]));

											break;

										case CPU_TO_KERNEL_END_PROC:

											log_info(logKernel,"recibi el tcb porque finalizo proceso por XXXX");

											break;

										case CPU_TO_KERNEL_BLOQUEO_INNN:

											log_info(logKernel,"ENTRE AL IF CON INNN");

											int32_t valor;

											printf("Ingrese un valor");
											scanf("%d",&valor);

											t_contenido mensaje;
											memset(mensaje,0,sizeof(t_contenido));
											strcpy(mensaje, string_from_format("[%d]",valor));
											enviarMensaje(i,KERNEL_TO_CPU_OK,mensaje,logKernel);

											break;

										case CPU_TO_KERNEL_BLOQUEO_INNC:
											log_info(logKernel,"ENTRE AL IF CON INNC");


											int32_t B = atoi(array[0]);

											char* cadena = malloc(B);


											printf("Ingrese una cadena de caracteres menor a %d",B);
											scanf("%s",cadena);

											log_info(logKernel,"el valor de la cadena es %s",cadena);

											t_contenido mensaje_para_cpu;
											memset(mensaje_para_cpu,0,B);
											memcpy(mensaje_para_cpu,cadena,B);
											enviarMensaje(i,KERNEL_TO_CPU_OK,mensaje_para_cpu,logKernel);
											break;

										case CPU_TO_KERNEL_BLOQUEO_OUTN :



											log_info(logKernel,"el valor del registro A es %d",atoi(array[0]));

											break;

										default:
											;
										}

									}
								}
							}
						}

						return NULL;
					}

void eliminarCpu(int32_t socketCpu) {

	// funcion criterio para buscar cpu por socket
	bool _match_cpu_fd(void* element){
		if(((t_client_cpu*)element)->cpuFD == socketCpu){
			return true;
		}
		return false;
	}

	// elimino la cpu de la lista de cpu's
	pthread_mutex_lock(&mutex_cpu_list);
		t_client_cpu* cpu = list_find(cpu_client_list, (void*)_match_cpu_fd);
		int32_t processPID = cpu->ocupado ? cpu->cpuPID : 0;
		int32_t cpuPID = cpu->cpuPID;
		list_remove_and_destroy_by_condition(cpu_client_list, (void*)_match_cpu_fd, (void*)free);
	pthread_mutex_unlock(&mutex_cpu_list);

	// ahora debería eliminar el proceso que estaba ejecutando, DEBERÍA estar en la cola de ejecutados
	log_info(logKernel, string_from_format("El hilo de planificador dice (?) : Alguien mató al CPU (PID:%d)", cpuPID));

	if(processPID != 0){
		removeProcess(processPID, false);
	}

}


void agregarCpu(int32_t socketCpu, char* mensaje){

	//Creo la estructura del nuevo CPU con todos sus datos
	t_client_cpu* aCPU = malloc(sizeof(t_client_cpu));
	aCPU->cpuFD = socketCpu;
	aCPU->cpuPID = atoi(mensaje);
	aCPU->socketProceso = 0;
	aCPU->pidTCB = 0;
	aCPU->tidTCB = 0;
	aCPU->ocupado = false;

	log_info(logKernel, string_from_format("PCP Thread says: Nuevo CPU conectado (PID: %d) aguarda instrucciones", aCPU->cpuPID));
	//Agrego el nuevo CPU a la lista.
	pthread_mutex_lock(&mutex_cpu_list);
		list_add(cpu_client_list, aCPU);
	pthread_mutex_unlock(&mutex_cpu_list);

}

