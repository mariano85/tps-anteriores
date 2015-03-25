/*
 * planificador.c
 *
 *  Created on: 17/10/2014
 *      Author: utnso
 */

#include "kernel.h"

void* planificador(t_thread *planificadorThread) {

	int myPID = process_get_thread_id();
	log_info(logKernel, "************** Comienza el planificador!(PID: %d) ***************",myPID);

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

					t_contenido mensaje_de_la_cpu; //buffer para el dato del cliente
					memset(mensaje_de_la_cpu, 0, sizeof(t_contenido));
					t_header header = recibirMensaje(i, mensaje_de_la_cpu, logKernel);
					if(strlen(mensaje_de_la_cpu) == 0){
						strcpy(mensaje_de_la_cpu, "[0]");
					}
					char** array = string_get_string_as_array(mensaje_de_la_cpu);

					switch (header) {
					case (ERR_CONEXION_CERRADA):{

						// cerramos socket y lo borramos del master set
						close(i);
						FD_CLR(i, &master);

						// eliminamos la cpu de la lista de cpu's y el proceso que estaba ejecutando, si hubiere
						eliminarCpu(i);
					}

						break;
					case ERR_ERROR_AL_RECIBIR_MSG:
						// TODO y aca que hacemos?
						break;

					case (CPU_TO_KERNEL_HANDSHAKE):{

						agregarCpu(i, mensaje_de_la_cpu);
						enviarMensaje(i, CPU_TO_KERNEL_HANDSHAKE, string_from_format("[%s,%d]",config_kernel.QUANTUM,config_kernel.TAMANIO_STACK), logKernel);
						agregarProcesoColaExec();

					}

						break;

					case (CPU_TO_KERNEL_FINALIZO_QUANTUM_NORMALMENTE):{

						//Aca hay que guardarlo en un estructura TCB y mandarlo a la cola de ready pero no se cual estructura usan te lo dejo a vos lean
						//MUESTRO LO QUE DEBERIA LLEGARTE IGUAL

						log_info(logKernel,"PID ES %d, Program_counter es %d ,modo es %d",atoi(array[0]),atoi(array[1]),atoi(array[2]));
						log_info(logKernel,"Los registros son : A es %d B es %d C es %d D es %d E es %d",atoi(array[3]),atoi(array[4]),atoi(array[5]),atoi(array[6]),atoi(array[7]));

//						t_process* aProcess = desocuparCPU(i);
						t_client_cpu *unaCpu = buscarCPUPorFD(i);
						t_process* aProcess = unaCpu->procesoExec;
						unaCpu->procesoExec = NULL;
						unaCpu->libre = true;
						log_info(logKernel, string_from_format("El hilo del Planificador dice: Cpu libre, espera instrucciones (PID: %d) aguarda instrucciones", unaCpu->cpuPID));

						if(aProcess == NULL){
							log_debug(logKernel, "\n\nel proceso liberado es %s\n", aProcess->nombre);
						} else {
							t_client_cpu *cpu = buscarCPUPorFD(i);
							log_debug(logKernel, "\n\nSOSPECHOSO: el proceso que ejecutaba la cpu es NULL. CPU ocupada? %s\n", cpu->libre ? "no" : "si");
						}

						actualizarTCB(aProcess, mensaje_de_la_cpu);

						agregarProcesoColaReady(aProcess);
						agregarProcesoColaExec();

					}

						break;

					case (CPU_TO_KERNEL_END_PROC):{
						log_info(logKernel,"recibi el tcb porque por una instruccion XXXX");

//						t_process* aProcess = desocuparCPU(i);
						t_client_cpu *unaCpu = buscarCPUPorFD(i);
						t_process* aProcess = unaCpu->procesoExec;
						unaCpu->procesoExec = NULL;
						unaCpu->libre = true;
						log_info(logKernel, string_from_format("El hilo del Planificador dice: Cpu libre, espera instrucciones (PID: %d) aguarda instrucciones", unaCpu->cpuPID));

						if(atoi(array[2])){ // es tcb de kernel

							procesoKernel->tcb->registros[0] = atoi(array[8]);
							procesoKernel->tcb->registros[1] = atoi(array[9]);
							procesoKernel->tcb->registros[2] = atoi(array[10]);
							procesoKernel->tcb->registros[3] = atoi(array[11]);
							procesoKernel->tcb->registros[4] = atoi(array[12]);

							aProcess = context_switch_vuelta();

							if(aProcess != NULL){
								agregarProcesoColaReady(aProcess);
							}
						} else { // es tcb de usuario

							actualizarTCB(aProcess, mensaje_de_la_cpu);
							enviarMensaje(aProcess->tcb->pid, KERNEL_TO_PRG_END_PRG, mensaje_de_la_cpu, logKernel);
							agregarProcesoColaExit(aProcess, EXIT);
						}

						agregarProcesoColaExec();

					}
						break;

					case (CPU_TO_KERNEL_INTERRUPCION):{ // sacar la funcion afuera
						uint32_t direccion = atoi(array[13]);

						t_process* procesoExec = traer_CPU_INTERRUPCION(i);

						actualizarTCB(procesoExec, mensaje_de_la_cpu);
						agregarProcesoColaSyscall(procesoExec, direccion);

						// el procesador está disponible y hay necesidad de ejecutar un syscall
						// Y el tcb km está tambien disponible para ejecutar!
						if(procesoKernel->tcb->cola == BLOCK && queue_size(COLA_SYSCALLS) > 0) {
							context_switch_ida();
							log_debug(logKernel, "\n\nvemos si estamos mandando el tcb km a ready %d\n", procesoKernel->tcb->pid);
							agregarProcesoColaReady(procesoKernel);
						}

						desocuparCPU_INTERRUPCION(i);

						agregarProcesoColaExec();

					}

						break;

					case (CPU_TO_KERNEL_ENTRADA_ESTANDAR):{

						log_info(logKernel,"Entre al if de la entrada estandar");
						entrada_estandar(i, mensaje_de_la_cpu);

					}

						break;

					case (CPU_TO_KERNEL_SALIDA_ESTANDAR):{

						log_info(logKernel,"Entre al if de la salida estandar");
						salida_estandar(i, mensaje_de_la_cpu);

					}

						break;

					case (CPU_TO_KERNEL_CREAR_HILO):{

						crear_hilo(mensaje_de_la_cpu);
						agregarProcesoColaExec();

					}

						break;
					case (CPU_TO_KERNEL_JOIN):{
						// TODO: recordar que aca cuando finalice el hilo a esperar mando a ready al hilo que espera
						join(i, mensaje_de_la_cpu);
					}

						break;
					case (CPU_TO_KERNEL_BLOCK):{
						bloquear(mensaje_de_la_cpu);
					}
						break;
					case (CPU_TO_KERNEL_WAKE):{
						despertar(mensaje_de_la_cpu);
					}
						break;

					case(MSP_TO_CPU_DIRECCION_INVALIDA):{


						t_client_cpu *unaCpu = buscarCPUPorFD(i);
						t_process* aProcess = unaCpu->procesoExec;
						unaCpu->procesoExec = NULL;
						memset(mensaje_de_la_cpu,0,sizeof(t_contenido));
						enviarMensaje(aProcess->tcb->pid,MSP_TO_CPU_DIRECCION_INVALIDA,mensaje_de_la_cpu,logKernel);
						agregarProcesoColaExit(aProcess, EXIT);

					}break;

					case(MSP_TO_CPU_VIOLACION_DE_SEGMENTO) : {

						t_client_cpu *unaCpu = buscarCPUPorFD(i);
						t_process* aProcess = unaCpu->procesoExec;
						unaCpu->procesoExec = NULL;
						memset(mensaje_de_la_cpu,0,sizeof(t_contenido));
						enviarMensaje(aProcess->tcb->pid,MSP_TO_CPU_VIOLACION_DE_SEGMENTO,mensaje_de_la_cpu,logKernel);
						agregarProcesoColaExit(aProcess, EXIT_ERROR);

					}break;

					case(MSP_TO_CPU_PID_INVALIDO) : {

						t_client_cpu *unaCpu = buscarCPUPorFD(i);
						t_process* aProcess = unaCpu->procesoExec;
						unaCpu->procesoExec = NULL;
						memset(mensaje_de_la_cpu,0,sizeof(t_contenido));
						enviarMensaje(aProcess->tcb->pid,MSP_TO_CPU_PID_INVALIDO,mensaje_de_la_cpu,logKernel);
						agregarProcesoColaExit(aProcess, EXIT_ERROR);

					}break;

					case(MSP_TO_CPU_MEMORIA_INSUFICIENTE) : {


						t_client_cpu *unaCpu = buscarCPUPorFD(i);
						t_process* aProcess = unaCpu->procesoExec;
						unaCpu->procesoExec = NULL;
						memset(mensaje_de_la_cpu,0,sizeof(t_contenido));
						enviarMensaje(aProcess->tcb->pid,MSP_TO_CPU_PID_INVALIDO,mensaje_de_la_cpu,logKernel);
						agregarProcesoColaExit(aProcess, EXIT_ERROR);

					}break;

					case(MSP_TO_CPU_TAMANIO_NEGATIVO) : {

						t_client_cpu *unaCpu = buscarCPUPorFD(i);
						t_process* aProcess = unaCpu->procesoExec;
						unaCpu->procesoExec = NULL;
						memset(mensaje_de_la_cpu,0,sizeof(t_contenido));
						enviarMensaje(aProcess->tcb->pid,MSP_TO_CPU_TAMANIO_NEGATIVO,mensaje_de_la_cpu,logKernel);
						agregarProcesoColaExit(aProcess, EXIT_ERROR);

					}break;

					case(MSP_TO_CPU_SEGMENTO_EXCEDE_TAMANIO_MAXIMO) : {

						t_client_cpu *unaCpu = buscarCPUPorFD(i);
						t_process* aProcess = unaCpu->procesoExec;
						unaCpu->procesoExec = NULL;
						memset(mensaje_de_la_cpu,0,sizeof(t_contenido));
						enviarMensaje(aProcess->tcb->pid,MSP_TO_CPU_SEGMENTO_EXCEDE_TAMANIO_MAXIMO,mensaje_de_la_cpu,logKernel);
						agregarProcesoColaExit(aProcess, EXIT_ERROR);

					}break;

					case(MSP_TO_CPU_PID_EXCEDE_CANT_MAXIMA_DE_SEGMENTOS) : {

						t_client_cpu *unaCpu = buscarCPUPorFD(i);
						t_process* aProcess = unaCpu->procesoExec;
						unaCpu->procesoExec = NULL;
						memset(mensaje_de_la_cpu,0,sizeof(t_contenido));
						enviarMensaje(aProcess->tcb->pid,MSP_TO_CPU_PID_EXCEDE_CANT_MAXIMA_DE_SEGMENTOS,mensaje_de_la_cpu,logKernel);
						agregarProcesoColaExit(aProcess, EXIT_ERROR);

					}break;

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
		log_info(logKernel, string_from_format("El hilo de planificador dice (?) : Alguien mató al CPU (PID:%d)", cpu->cpuPID));

		// si estaba ejecutando un proceso, lo mando a la cola de EXIT
		if(cpu->procesoExec != NULL){

			if(cpu->procesoExec->tcb->kernel_mode){
				log_info(logKernel, "Si se muere el TCB del Kernel se muere todo. Au revoir!");
				finishKernel();
				exit(EXIT_FAILURE);
			}

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
	aCPU->libre = true;

	log_debug(logKernel, "procesos en ready %d", COLA_READY->elements->elements_count);
	log_debug(logKernel, "procesos en exec %d", list_count_satisfying(cpu_client_list, (void*)cpuOcupada));
	log_info(logKernel, string_from_format("Planificador Thread dice: Nuevo CPU conectado (PID: %d) aguarda instrucciones", aCPU->cpuPID));

	//Agrego el nuevo CPU a la lista.
	pthread_mutex_lock(&mutex_cpu_list);
		list_add(cpu_client_list, aCPU);
	pthread_mutex_unlock(&mutex_cpu_list);

}


void actualizarTCB(t_process* aProcess, char* mensaje) {

	char** split = string_get_string_as_array(mensaje);

	aProcess->tcb->puntero_instruccion = atoi(split[5]);
	aProcess->tcb->cursor_stack = atoi(split[7]);
	aProcess->tcb->registros[0] = atoi(split[8]);
	aProcess->tcb->registros[1] = atoi(split[9]);
	aProcess->tcb->registros[2] = atoi(split[10]);
	aProcess->tcb->registros[3] = atoi(split[11]);
	aProcess->tcb->registros[4] = atoi(split[12]);

}


// TODO: esta funcion tampoco se usa acá
void manejarFinDeQuantum(int32_t socketCpu, char* mensaje){

	log_debug(logKernel, "manejo el fin de quantum...");
	t_process* aProcess = desocuparCPU(socketCpu);
	log_debug(logKernel, "\n\nel proceso liberado es %s\n", aProcess->nombre);

	actualizarTCB(aProcess, mensaje);

	agregarProcesoColaReady(aProcess);

}


// TODO: esta funcion no se usa, se mandó toda la lógica al switch grande
void manejarFinDeProceso(int32_t socketCpu, char* mensajeRecibido) {

	log_debug(logKernel, "manejo el fin del proceso...");
	t_process*aProcess = desocuparCPU(socketCpu);

	log_info(logKernel,"pase acaaaa");

	actualizarTCB(aProcess, mensajeRecibido);

	if(aProcess->tcb->kernel_mode){
		// terminó de ejecutar el tcb km
		aProcess = context_switch_vuelta();

		if(aProcess != NULL){
			agregarProcesoColaReady(aProcess);
		}

	} else {

		char** split = string_get_string_as_array(mensajeRecibido);
		t_contenido mensaje;
		if((split[1] != NULL) && !(strlen(split[1]))==0){
			memset(mensaje, 0, sizeof(t_contenido));
			// TODO: vemos aca que le mandamos a la consola y que es lo que tiene que imprimir.-
			strcpy(mensaje, split[1]);
			enviarMensaje(aProcess->tcb->pid, KERNEL_TO_PRG_END_PRG, mensaje, logKernel);
		}

		agregarProcesoColaExit(aProcess, EXIT);
		log_debug(logKernel, "procesos en ready %d", COLA_READY->elements->elements_count);
		log_debug(logKernel, "procesos en exec %d", list_count_satisfying(cpu_client_list, (void*)cpuOcupada));
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
 * TODO: WARNING! TRATAR DE NO INVOCAR ESTA FUNCION EN EL SWITCH GRANDE
 */
t_process* desocuparCPU(int32_t socketCpu) {

	t_client_cpu *unaCpu = buscarCPUPorFD(socketCpu);
	t_process* unProceso = unaCpu->procesoExec;
	unaCpu->procesoExec = NULL;
	unaCpu->libre = true;
	log_info(logKernel, string_from_format("El hilo del Planificador dice: Cpu libre, espera instrucciones (PID: %d) aguarda instrucciones", unaCpu->cpuPID));
	return unProceso;

}


t_process* traer_CPU_INTERRUPCION(int32_t socketCpu){

	t_client_cpu *unaCpu = buscarCPUPorFD(socketCpu);
	t_process* unProceso = unaCpu->procesoExec;
	log_info(logKernel, string_from_format("El hilo del Planificador dice: Cpu libre, espera instrucciones (PID: %d) aguarda instrucciones", unaCpu->cpuPID));
	return unProceso;
}

t_process* desocuparCPU_INTERRUPCION(int32_t socketCPU){

	t_client_cpu *unaCpu = buscarCPUPorFD(socketCPU);
	t_process* unProceso = unaCpu->procesoExec;
	unaCpu->procesoExec = NULL;
	unaCpu->libre = true;
	log_info(logKernel, string_from_format("Saco el Proceso :  (PID: %d) aguarda instrucciones", unaCpu->cpuPID));
	return unProceso;


}


void context_switch_ida(){

	t_process* aProcess = queue_peek(COLA_SYSCALLS);

	if(aProcess->tcb->pid == KERNEL_PID){
		log_debug(logKernel, "\n\npor que cazzo este está en -1???\n");
	}

	pthread_mutex_lock(&mutex_tcb_km);
		procesoKernel->tcb->pid = aProcess->tcb->pid;
		procesoKernel->tcb->tid = aProcess->tcb->tid;
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
//		aProcess = queue_peek(COLA_SYSCALLS);
//
//		if(aProcess == NULL && aProcess->tcb->pid == procesoKernel->tcb->pid && aProcess->tcb->tid == procesoKernel->tcb->tid){
			aProcess = queue_pop(COLA_SYSCALLS);
//		}
	pthread_mutex_unlock(&mutex_syscalls_queue);

	if(aProcess == NULL){
		return NULL;
	}

	bool _es_tcb_km(void* element){
		t_hilo* ele = ((t_process*)element)->tcb;
		return ele->pid == KERNEL_PID;
	}

	bool hay = list_any_satisfy(COLA_READY->elements, (void*)_es_tcb_km);

	log_debug(logKernel, "\n\nel tcb km está en la cola de ready? %s\n", hay ? "SI" : "NO");

	//	aProcess->tcb->pid = procesoKernel->tcb->pid;
	//	aProcess->tcb->tid = procesoKernel->tcb->tid;
	aProcess->tcb->registros[0] = procesoKernel->tcb->registros[0];
	aProcess->tcb->registros[1] = procesoKernel->tcb->registros[1];
	aProcess->tcb->registros[2] = procesoKernel->tcb->registros[2];
	aProcess->tcb->registros[3] = procesoKernel->tcb->registros[3];
	aProcess->tcb->registros[4] = procesoKernel->tcb->registros[4];

	pthread_mutex_lock(&mutex_tcb_km);
		procesoKernel->tcb->cola = BLOCK;
		procesoKernel->tcb->pid = KERNEL_PID;
		procesoKernel->tcb->tid = KERNEL_TID;
	pthread_mutex_unlock(&mutex_tcb_km);

	list_remove_by_condition(COLA_READY->elements, (void*)_es_tcb_km);

	return aProcess;
}


// TODO: esta funcion no se usa, se mandó toda la lógica al switch grande
void interrupcion(int32_t socketCpu, char* mensaje){

	char** split = string_get_string_as_array(mensaje);
	uint32_t direccion = atoi(split[13]);
	t_process* procesoExec = desocuparCPU(socketCpu);

	actualizarTCB(procesoExec, mensaje);

	agregarProcesoColaSyscall(procesoExec, direccion);

	// el procesador está disponible y hay necesidad de ejecutar un syscall
	// Y el tcb km está tambien disponible para ejecutar!
	if(procesoKernel->tcb->cola == BLOCK && queue_size(COLA_SYSCALLS) > 0) {
		context_switch_ida();

		log_debug(logKernel, "\n\nvemos si estamos mandando el tcb km a ready %d\n", procesoKernel->tcb->pid);
		agregarProcesoColaReady(procesoKernel);
	}

}


void entrada_estandar(int32_t socketCpu, char* mensajeRecibido){

	char** array = string_get_string_as_array(mensajeRecibido);
	t_process* aProcess = desocuparCPU(socketCpu);

	log_info(logKernel, "Esto tiene que ser true %s", aProcess->tcb->pid == atoi(array[0]) ? "true" : "false");

	enviarMensaje(aProcess->tcb->pid, KERNEL_TO_CONSOLA_ENTRADA_ESTANDAR, mensajeRecibido, logKernel);

	t_contenido mensaje;
	memset(mensaje,0,sizeof(mensaje));
	recibirMensaje(aProcess->tcb->pid,mensaje, logKernel);

	enviarMensaje(socketCpu, KERNEL_TO_CPU_OK, mensaje, logKernel);
}


void salida_estandar(int32_t socketCpu, char* mensajeRecibido){

	t_process* aProcess = desocuparCPU(socketCpu);
	agregarProcesoColaReady(aProcess);
	enviarMensaje(aProcess->tcb->pid, KERNEL_TO_CONSOLA_SALIDA_ESTANDAR, mensajeRecibido, logKernel);

}


void crear_hilo(char* mensaje){

	t_process* proceso = getProcesoDesdeMensaje(mensaje);

	if(proceso != NULL){
		agregarProcesoColaReady(proceso);
	} else {
		// comunico al padre que no se pudo crear su hilo
	}

}


void join(int32_t socketCpu, char* mensajeRecibido){

	// asumo que el pid al que pertenecen ambos hilos son del proceso que se está ejecutando en la cpu
	// nah, la cpu me manda 3 parametros y listo ;)
	t_process *proceso_llamador = NULL, *proceso_a_esperar = NULL;
	char** array = string_get_string_as_array(mensajeRecibido);
	int32_t pid = atoi(array[0]);
	int32_t tid_llamador = atoi(array[1]);
	int32_t tid_a_esperar = atoi(array[2]);

	pthread_mutex_lock(&mutex_syscalls_queue);
	// pongo las manos en el fuego que el peek() de la cola de syscalls es el proceso que invocó dicho servicio
	// validarlo por las dudas
		proceso_llamador = queue_pop(COLA_SYSCALLS);
	pthread_mutex_unlock(&mutex_syscalls_queue);

	pthread_mutex_lock(&mutex_join_queue);
		queue_push(COLA_JOIN, proceso_llamador);
	pthread_mutex_unlock(&mutex_join_queue);

	proceso_a_esperar = encontrarProcesoPorPIDyTID(pid, tid_a_esperar);
	proceso_a_esperar->tid_llamador_join = tid_llamador;

}


void bloquear(char* mensajeRecibido){

	char** split = string_get_string_as_array(mensajeRecibido);
	char* id_recurso = split[13];
	t_process* proceso_a_bloquear = NULL;

	pthread_mutex_lock(&mutex_syscalls_queue);
		proceso_a_bloquear = queue_pop(COLA_SYSCALLS);
	pthread_mutex_unlock(&mutex_syscalls_queue);

	t_queue* cola_bloqueados = dictionary_get(mapa_recursos, id_recurso);

	if(cola_bloqueados == NULL){
		// notificar que el recurso no existe, pero se crea =)
		cola_bloqueados = queue_create();
		dictionary_put(mapa_recursos, id_recurso, cola_bloqueados);
	}

	// aca podriamos poner un mutexcito
	if(proceso_a_bloquear != NULL){
		queue_push(cola_bloqueados, proceso_a_bloquear);
	}

}


void despertar(char* mensajeRecibido){

	char** split = string_get_string_as_array(mensajeRecibido);
	char* id_recurso = split[13];
	t_process* proceso_a_despertar = NULL;

	t_queue* cola_bloqueados = dictionary_get(mapa_recursos, id_recurso);

	if(cola_bloqueados != NULL){

		// aca tambien podriamos poner un mutexcito
		proceso_a_despertar = queue_pop(cola_bloqueados);

		agregarProcesoColaReady(proceso_a_despertar);
	} else {
		// notificar que ese recurso no existe
	}

}
