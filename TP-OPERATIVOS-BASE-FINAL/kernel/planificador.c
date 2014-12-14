/*
 * planificador.c
 *
 *  Created on: 17/10/2014
 *      Author: utnso
 */

#include "kernel.h"

void* planificador(t_thread *planificadorThread) {

	int myPID = process_get_thread_id();
	log_info(logPlanificador, "************** Comienza el planificador!(PID: %d) ***************",myPID);

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
						log_error(logPlanificador, string_from_format( "Hubo un error en el accept para el fd: %i", i));
					} else {
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}

						//Shows the new connection administrated
						log_info(logPlanificador,
								string_from_format(
										"selectserver: new connection from %s on socket %d\n",
										inet_ntop(remoteaddr.ss_family,
												get_in_addr(
														(struct sockaddr*) &remoteaddr),
												remoteIP, INET6_ADDRSTRLEN),
										newfd));
					}

				} else {

					t_contenido mensaje_de_la_cpu; //buffer para el dato del cliente
					memset(mensaje_de_la_cpu, 0, sizeof(t_contenido));
					t_header header = recibirMensaje(i, mensaje_de_la_cpu, logPlanificador);
					if(strlen(mensaje_de_la_cpu) == 0){
						strcpy(mensaje_de_la_cpu, "[0]");
					}
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

					case CPU_TO_KERNEL_HANDSHAKE:

						agregarCpu(i, mensaje_de_la_cpu);
						agregarProcesoColaExec();

						break;
					case CPU_TO_KERNEL_FINALIZO_QUANTUM_NORMALMENTE:

						//Aca hay que guardarlo en un estructura TCB y mandarlo a la cola de ready pero no se cual estructura usan te lo dejo a vos lean
						//MUESTRO LO QUE DEBERIA LLEGARTE IGUAL

						log_info(logPlanificador,"PID ES %d, Program_counter es %d ,modo es %d",atoi(array[0]),atoi(array[1]),atoi(array[2]));
						log_info(logPlanificador,"Los registros son : A es %d B es %d C es %d D es %d E es %d",atoi(array[3]),atoi(array[4]),atoi(array[5]),atoi(array[6]),atoi(array[7]));

						manejarFinDeQuantum(i, mensaje_de_la_cpu);
						agregarProcesoColaExec();

						break;

					case CPU_TO_KERNEL_END_PROC:

						log_info(logPlanificador,"recibi el tcb porque finalizo proceso por XXXX");

						manejarFinDeProceso(i, mensaje_de_la_cpu);
						agregarProcesoColaExec();

						break;

					case CPU_TO_KERNEL_INTERRUPCION:

						interrupcion(i, mensaje_de_la_cpu);
						agregarProcesoColaExec();

						break;

					case CPU_TO_KERNEL_ENTRADA_ESTANDAR:

						entrada_estandar(i, mensaje_de_la_cpu);

						break;

					case CPU_TO_KERNEL_SALIDA_ESTANDAR:

						log_info(logPlanificador,"Entre al if de la salida estandar");

						salida_estandar(i, mensaje_de_la_cpu);

						break;

					case CPU_TO_KERNEL_CREAR_HILO:

						crear_hilo(mensaje_de_la_cpu);
						agregarProcesoColaExec();

						break;
					case CPU_TO_KERNEL_JOIN:
						// TODO: recordar que aca cuando finalice el hilo a esperar mando a ready al hilo que espera
						break;
					case CPU_TO_KERNEL_BLOCK:
						break;
					case CPU_TO_KERNEL_WAKE:
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


/*
 * aca no invoco a las funciones de busqueda para aprovechar mejor el semáforo
 */
void eliminarCpu(int32_t socketCpu) {

	// funcion criterio para buscar cpu por socket
	bool _match_cpu_fd(void* element){
		return ((t_client_cpu*)element)->cpuFD == socketCpu;
	}

	// elimino la cpu de la lista de cpu's
	// TODO: y si no se encuentra la cpu?
	pthread_mutex_lock(&mutex_cpu_list);
		t_client_cpu* cpu = list_find(cpu_client_list, (void*)_match_cpu_fd);

		// ahora debería eliminar el proceso que estaba ejecutando, DEBERÍA estar en la cola de ejecutados
		log_info(logPlanificador, string_from_format("El hilo de planificador dice (?) : Alguien mató al CPU (PID:%d)", cpu->cpuPID));

		// si estaba ejecutando un proceso, lo mando a la cola de EXIT
		if(cpu->procesoExec != NULL){
			agregarProcesoColaExit(cpu->procesoExec, EXIT_ABORT_CPU);
		}

		list_remove_and_destroy_by_condition(cpu_client_list, (void*)_match_cpu_fd, (void*)free);
	pthread_mutex_unlock(&mutex_cpu_list);

}


void agregarCpu(int32_t socketCpu, char* mensaje){

	//Creo la estructura del nuevo CPU con todos sus datos
	t_client_cpu* aCPU = malloc(sizeof(t_client_cpu));
	aCPU->cpuFD = socketCpu;
	aCPU->cpuPID = socketCpu;
	aCPU->procesoExec = NULL;

	log_info(logPlanificador, string_from_format("Planificador Thread dice: Nuevo CPU conectado (PID: %d) aguarda instrucciones", aCPU->cpuPID));
	//Agrego el nuevo CPU a la lista.
	pthread_mutex_lock(&mutex_cpu_list);
		list_add(cpu_client_list, aCPU);
	pthread_mutex_unlock(&mutex_cpu_list);

}


void actualizarTCB(t_process* aProcess, char* mensaje) {

	char** split = string_get_string_as_array(mensaje);

	aProcess->tcb->puntero_instruccion = atoi(split[1]);
	aProcess->tcb->registros[0] = atoi(split[3]);
	aProcess->tcb->registros[1] = atoi(split[4]);
	aProcess->tcb->registros[2] = atoi(split[5]);
	aProcess->tcb->registros[3] = atoi(split[6]);
	aProcess->tcb->registros[4] = atoi(split[7]);

}


void manejarFinDeQuantum(int32_t socketCpu, char* mensaje){

	t_process* aProcess = desocuparCPU(socketCpu);

	actualizarTCB(aProcess, mensaje);

	agregarProcesoColaReady(aProcess);

}


void manejarFinDeProceso(int32_t socketCpu, char* mensajeRecibido) {

	t_contenido mensaje;
	char** split = string_get_string_as_array(mensajeRecibido);

	t_process* aProcess = desocuparCPU(socketCpu);

	actualizarTCB(aProcess, mensajeRecibido);

	if(aProcess->tcb->kernel_mode){
		// terminó de ejecutar el tcb km
		aProcess = context_switch_vuelta();

		agregarProcesoColaReady(aProcess);

	} else {

		agregarProcesoColaExit(aProcess, EXIT);

		if((split[1] != NULL) && !(strlen(split[1]))==0){
			memset(mensaje, 0, sizeof(t_contenido));
			// TODO: vemos aca que le mandamos a la consola y que es lo que tiene que imprimir.-
			strcpy(mensaje, split[1]);
			enviarMensaje(aProcess->process_fd, KERNEL_TO_PRG_IMPR_VARIABLES, mensaje, logPlanificador);
		}
	}

}


t_client_cpu* buscarCPUPorFD(int32_t socketCpu) {

	t_client_cpu *unaCpu = NULL;

	bool _match_tcb(void* element){
		t_client_cpu* ele = (t_client_cpu*)element;
		return ele->cpuFD == socketCpu;
	}

	// busco el proceso que estaba ejecutando
	pthread_mutex_lock(&mutex_cpu_list);
		unaCpu = list_find(cpu_client_list, (void*)_match_tcb);
	pthread_mutex_unlock(&mutex_cpu_list);

	return unaCpu;
}


/*
 * si obtengo el proceso ejecutandose en la cpu
 * marco a la cpu como desocupada
 */
t_process* desocuparCPU(int32_t socketCpu) {

	t_client_cpu *unaCpu = buscarCPUPorFD(socketCpu);
	t_process* unProceso = unaCpu->procesoExec;
	unaCpu->procesoExec = NULL;
	log_info(logPlanificador, string_from_format("El hilo del Planificador dice: Cpu libre, espera instrucciones (PID: %d) aguarda instrucciones", unaCpu->cpuPID));
	return unProceso;

}


void context_switch_ida(){

	t_process* aProcess = queue_peek(COLA_SYSCALLS);

	pthread_mutex_lock(&mutex_tcb_km);
		procesoKernel->tcb->pid = aProcess->tcb->pid;
		procesoKernel->tcb->registros[0] = aProcess->tcb->registros[0];
		procesoKernel->tcb->registros[1] = aProcess->tcb->registros[1];
		procesoKernel->tcb->registros[2] = aProcess->tcb->registros[2];
		procesoKernel->tcb->registros[3] = aProcess->tcb->registros[3];
		procesoKernel->tcb->registros[4] = aProcess->tcb->registros[4];
		procesoKernel->tcb->puntero_instruccion = aProcess->direccion_syscall;
		procesoKernel->tcb->cola = READY;
	pthread_mutex_unlock(&mutex_tcb_km);

}


t_process* context_switch_vuelta(){

	t_process* aProcess = NULL;

	pthread_mutex_lock(&mutex_syscalls_queue);
		aProcess = queue_pop(COLA_SYSCALLS);
	pthread_mutex_unlock(&mutex_syscalls_queue);

	aProcess->tcb->registros[0] = procesoKernel->tcb->registros[0];
	aProcess->tcb->registros[1] = procesoKernel->tcb->registros[1];
	aProcess->tcb->registros[2] = procesoKernel->tcb->registros[2];
	aProcess->tcb->registros[3] = procesoKernel->tcb->registros[3];
	aProcess->tcb->registros[4] = procesoKernel->tcb->registros[4];

	pthread_mutex_lock(&mutex_tcb_km);
		procesoKernel->tcb->cola = BLOCK;
		procesoKernel->tcb->pid = KERNEL_PID;
	pthread_mutex_unlock(&mutex_tcb_km);

	return aProcess;
}


void interrupcion(int32_t socketCpu, char* mensaje){

	char** split = string_get_string_as_array(mensaje);
	uint32_t direccion = atoi(split[9]);
	t_process* procesoExec = desocuparCPU(socketCpu);

	actualizarTCB(procesoExec, mensaje);

	agregarProcesoColaSyscall(procesoExec, direccion);

	// el procesador está disponible y hay necesidad de ejecutar un syscall
	// Y el tcb km está tambien disponible para ejecutar!
	if(procesoKernel->tcb->cola == BLOCK && queue_size(COLA_SYSCALLS) > 0) {
		context_switch_ida();
		agregarProcesoColaReady(procesoKernel);
	}

}


void entrada_estandar(int32_t socketCpu, char* mensajeRecibido){

	char** array = string_get_string_as_array(mensajeRecibido);
	t_process* aProcess = desocuparCPU(socketCpu);

	log_info(logPlanificador, "Esto tiene que ser true %s", aProcess->tcb->pid == atoi(array[0]) ? "true" : "false");

	enviarMensaje(aProcess->process_fd, KERNEL_TO_CONSOLA_ENTRADA_ESTANDAR, array[1], logPlanificador);

	t_contenido mensaje;
	memset(mensaje,0,sizeof(mensaje));
	recibirMensaje(aProcess->process_fd,mensaje, logPlanificador);

	enviarMensaje(socketCpu, KERNEL_TO_CPU_OK, mensaje, logPlanificador);
}


void salida_estandar(int32_t socketCpu, char* mensajeRecibido){

	char** array = string_get_string_as_array(mensajeRecibido);
	t_process* aProcess = desocuparCPU(socketCpu);

	enviarMensaje(aProcess->process_fd, KERNEL_TO_CONSOLA_SALIDA_ESTANDAR, array[1], logPlanificador);

}


void crear_hilo(char* mensaje){

	t_process* proceso = getProcesoDesdeMensaje(mensaje);

	if(proceso != NULL){
		agregarProcesoColaReady(proceso);
	} else {
		// comunico al padre que no se pudo crear su hilo
	}

}
