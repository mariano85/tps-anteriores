/*
 * loader.c
 *
 *  Created on: 11/10/2014
 *      Author: utnso
 */

#include "kernel.h"
t_log* logKernel;


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

void* loader(t_loaderThread *loaderThread){
	int myPID = process_get_thread_id();
	log_info(logKernel, "************** PLP Thread Started!(PID: %d) ***************", myPID);

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

						if(i == socketMSP){
							log_info(logKernel, "Wow! La MSP se desconectó! Imposible seguir funcionando! :/");
						}
						else if(stillInside(i)){
							int32_t processPID = encontrarProcesoPorFD(i);
							removeProcess(processPID, true);
							mostrarColas();
						}

						break;
					case ERR_ERROR_AL_RECIBIR_MSG:
						//TODO retry?l
						break;
					case CON_TO_KRN_HANDSHAKE:
						enviarMensaje(i, CON_TO_KRN_HANDSHAKE, "KERNEL - Handshake Response", logKernel);
						break;
					case CON_TO_KRN_CODE: {
						t_process *procesoNuevo = NULL;
						char** split = string_get_string_as_array(mensaje);
						int32_t programPID = atoi(split[0]);
						int32_t programTID = atoi(split[1]);

						char* stringCode = recibirCodigo(i, CON_TO_KRN_CODE, logKernel);

						// no es elegante, tengo que recibir desde el mensaje cuantos bytes deberia haber recibido...
						int32_t tamanioCodigo = strlen(stringCode);

						log_debug(logKernel, string_from_format( "Se recibio codigo completo del programa con FD: %i", i));
						log_debug(logKernel, string_from_format( "El codigo recibido es:\n %s \n", stringCode));

						procesoNuevo = getProcesoDesdeCodigoBESO(MODO_USUARIO, stringCode, tamanioCodigo, programPID, programTID, i);

						log_info(logKernel, "Se generó la estructura del proceso con éxito!");
						agregarProcesoColaNew(procesoNuevo);

						break;
					}
					default:
						;
					}
				} // END handle data from client
			} // END got new incoming connection
		} // END looping through file descriptors
	} // END for(;;)--and you thought it would never end!
	return NULL ;
}
