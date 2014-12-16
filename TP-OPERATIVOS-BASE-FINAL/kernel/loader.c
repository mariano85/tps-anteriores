/*
 * loader.c
 *
 *  Created on: 11/10/2014
 *      Author: utnso
 */

#include "kernel.h"
t_log* logKernel;

void grabarCodigoRecibido(char* codigoBeso, char* nombreArchivo, int32_t tamanio);

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

void* loader(t_thread *loaderThread){
	int tid = process_get_thread_id();
	log_info(logKernel, "************** El Thread del Loder comenzó!(PID: %d) ***************", tid);

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
	socketInfo.sin_port = htons(config_kernel.PUERTO);
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
	for (;;) {

		read_fds = master; // copy it

		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL ) == -1) {
			perror("select");
			exit(4);
		}

		// run through the existing connections looking for data to read
		for (i = 0; i <= fdmax; i++) {

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
					t_contenido mensaje; //buffer para el dato del cliente
					memset(mensaje, 0, sizeof(t_contenido));
					t_header header = recibirMensaje(i, mensaje, logKernel);

					switch (header) {
					case ERR_CONEXION_CERRADA:
						//Removes from master set and say good bye! :)
						close(i); // bye!

						FD_CLR(i, &master); // remove from master set

						t_process* aProcess = encontrarYRemoverProcesoPorFD(i);
						// agrego proceso a la cola de EXIT
						if(aProcess != NULL){
							log_info(logKernel, string_from_format( "Si llega hasta aca significa que la consola (PID = %d) tuvo una terminación anormal", i));
							agregarProcesoColaExit(aProcess, EXIT_ABORT_CON);
						}

						break;
					case ERR_ERROR_AL_RECIBIR_MSG:
						//TODO retry?l
						break;
					case PRG_TO_KRN_HANDSHAKE:
						enviarMensaje(i, PRG_TO_KRN_HANDSHAKE, "KERNEL - Handshake Response", logKernel);
						break;
					case PRG_TO_KRN_CODE: {
						char *codigoBESO = NULL;
						t_process *procesoNuevo = NULL;

						char** split = string_get_string_as_array(mensaje);
						char* nombreBESO = split[0];
						size_t tamanioCodigo = atoi(split[1]);

						codigoBESO = calloc(tamanioCodigo, 1);

						enviarMensaje(i, PRG_TO_KRN_CODE, "Se espera el codigo BESO...", logKernel);

						recibir(i, codigoBESO, tamanioCodigo);

						grabarCodigoRecibido(codigoBESO, "beso.bc", tamanioCodigo);

						procesoNuevo = getProcesoDesdeCodigoBESO(nombreBESO, MODO_USUARIO, codigoBESO, tamanioCodigo, i);

						free(codigoBESO);

						if(procesoNuevo != NULL){
							log_debug(logKernel, string_from_format( "Inserto en la cola de NEW el programa con FD: %i", i));
							agregarProcesoColaNew(procesoNuevo);
						} else {
							enviarMensaje(i, KERNEL_TO_PRG_NO_MEMORY, "", logKernel);
						}

						break;
					}
					default:
						;
					}
				}
			}
		}
	}
	return NULL;
}

char *recibirCodigoBeso(int32_t socketConsola, size_t tamanioCodigo){

	enviarMensaje(socketConsola, PRG_TO_KRN_CODE, "Se espera el codigo beso", logKernel);
	char *codigoBeso = calloc(1, tamanioCodigo);

	if( (recibir(socketConsola, codigoBeso, tamanioCodigo)) != EXITO_SOCK){
		free(codigoBeso);
		codigoBeso = NULL;
	}

	return codigoBeso;
}

void grabarCodigoRecibido(char* codigoBeso, char* nombreArchivo, int32_t tamanio){

	FILE* fp = fopen(nombreArchivo, "w+");
	fwrite(codigoBeso, tamanio, 1, fp);

}
