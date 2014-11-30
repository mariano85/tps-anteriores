/*
 * planificador.c
 *
 *  Created on: 21/11/2014
 *      Author: utnso
 */

#include "kernel.h"
t_log* kernelLog;

void eliminarCpu(int32_t socketCpu);
void agregarCpu(int32_t socketCpu, char* mensaje);
void manejarFinDeQuantum(int32_t socketCpu, char* mensaje);
void manejarFinDeProceso(int32_t socketCpu, char* mensaje);

/*
 * llamadas al sistema
 */
void manejar_INTE(int32_t socketCpu, char* mensaje);

void* planificador(t_thread *planificadorThread){

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

					t_contenido mensaje; //buffer para el dato del cliente
					memset(mensaje, 0, sizeof(t_contenido));
					t_header header = recibirMensaje(i, mensaje, logKernel);

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
					case CPU_TO_KERNEL_HANDSHAKE:

						// agrego cpu a la lista
						agregarCpu(i, mensaje);

						// muevo un proceso de READY -> EXEC y lo mando a ejecutar a la pc disponible (que deberia tener)
						agregarProcesoColaExec();

						break;
					case CPU_TO_KERNEL_FIN_QUANTUM: // EXEC -> READY

						manejarFinDeQuantum(i, mensaje);
						agregarProcesoColaExec();

						break;
					case CPU_TO_KERNEL_END_PROC: // EXEC -> EXIT

						manejarFinDeProceso(i, mensaje);
						agregarProcesoColaExec();

						break;
					case CPU_TO_KERNEL_INTE:

						manejar_INTE(i, mensaje);

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

void eliminarCpu(int32_t socketCpu){

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

void manejarFinDeQuantum(int32_t socketCpu, char* mensaje){

	char** split = string_get_string_as_array(mensaje);
	int32_t pid = atoi(split[0]);
	int32_t tid = atoi(split[8]);

	bool _match_tcb(void* element){
		t_process* unProceso = (t_process*)element;
		return unProceso->tcb->pid == pid && unProceso->tcb->tid == tid;
	}

	bool wasExecuting = false;

	//Busco el PCB que está en ejecución y lo encolo en lista de READY
	pthread_mutex_lock(&mutex_exec_queue);
		t_process* aProcess = list_find(EXEC->elements, (void*)_match_tcb);

		if(aProcess != NULL) {
			wasExecuting = true;

			// lo saco de la cola de EXEC
			list_remove_by_condition(EXEC->elements, (void*)_match_tcb);

			// actualizo el TCB con los valores que recibi de la CPU
			// TODO actualizo el kernel mode tambien? ver bien el mode switch!

			aProcess->tcb->program_counter = atoi(split[1]);
			aProcess->tcb->indicador_modo_kernel = atoi(split[2]);
			aProcess->tcb->registros_de_programacion->A = atoi(split[3]);
			aProcess->tcb->registros_de_programacion->B = atoi(split[4]);
			aProcess->tcb->registros_de_programacion->C = atoi(split[5]);
			aProcess->tcb->registros_de_programacion->D = atoi(split[6]);
			aProcess->tcb->registros_de_programacion->E = atoi(split[7]);

			agregarProcesoColaReady(aProcess);

		}
	pthread_mutex_unlock(&mutex_exec_queue);

	//Seteo como disponible al procesador si y solo si, ya no le di otro programa para ejecutar!
	pthread_mutex_lock(&mutex_cpu_list);
		t_client_cpu* aCPU = buscarCpuPorSocket(socketCpu);

		if(aCPU != NULL && ( (aCPU->ocupado && aCPU->pidTCB == pid && aCPU->tidTCB == tid) || wasExecuting) ){

			log_info(logKernel, string_from_format("El hilo del Planificador dice: Cpu libre, espera instrucciones (PID: %d) aguarda instrucciones", aCPU->cpuPID));

			aCPU->ocupado = false;
			aCPU->socketProceso = 0;
			aCPU->pidTCB = 0;
			aCPU->tidTCB = 0;
		}
	pthread_mutex_unlock(&mutex_cpu_list);

}


/*
 *
 */
void manejarFinDeProceso(int32_t socketCpu, char* mensajeRecibido) {

	char** split = string_get_string_as_array(mensajeRecibido);
	int32_t pid = atoi(split[0]);
	int32_t tid = atoi(split[8]);

	bool _match_tcb(void* element){
		t_process* unProceso = (t_process*)element;
		return unProceso->tcb->pid == pid && unProceso->tcb->tid == tid;
	}

	// busco el proceso que estaba ejecutando
	pthread_mutex_lock(&mutex_exec_queue);
		t_process* aProcess = list_find(EXEC->elements, (void*)_match_tcb);
		list_remove_by_condition(EXEC->elements, (void*)_match_tcb);
	pthread_mutex_unlock(&mutex_exec_queue);

	t_contenido mensaje;

	// TODO: ver que valida aca en los mensajes
	if((split[1] != NULL) && !(strlen(split[1]))==0){
		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, split[1]);
		// TODO ver aca
		enviarMensaje(aProcess->process_fd, KERNEL_TO_PRG_IMPR_VARIABLES, mensaje, logKernel);
	}

	agregarProcesoColaExit(aProcess);

	// dejo la cpu disponible
	pthread_mutex_lock(&mutex_cpu_list);
		t_client_cpu* aCPU = buscarCpuPorSocket(socketCpu);
		aCPU->ocupado = false;
		aCPU->socketProceso = 0;
		aCPU->pidTCB = 0;
		aCPU->tidTCB = 0;
	pthread_mutex_unlock(&mutex_cpu_list);

}


/*
 * vamos a hacer el manejo de la interrupcion aca. despues lo adaptamos a los servicios_kernel
 */

void manejar_INTE(int32_t socketCpu, char* mensaje){

	char** split = string_get_string_as_array(mensaje);
	int32_t pid = atoi(split[0]);
	int32_t tid = atoi(split[8]);
	int32_t direccion = atoi(split[9]);




}


