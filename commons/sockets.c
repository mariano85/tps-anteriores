
#include "sockets.h"

int crearSocket() {

	int newSocket;
	int si = 1;
	if ((newSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return EXIT_FAILURE;
	} else {
		if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, &si, sizeof(int)) == -1) {
			return EXIT_FAILURE;
		}
		return newSocket;
	}
}


int bindearSocket(int newSocket, tSocketInfo socketInfo) {
	if (bind(newSocket, (struct sockaddr*)&socketInfo, sizeof(socketInfo)) == -1) {
		perror("Error al bindear socket escucha");
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}


int escucharEn(int newSocket) {
	if (listen(newSocket, BACKLOG) == -1) {
		perror("Error al poner a escuchar socket");
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}


int conectarAServidor(char *ipDestino, unsigned short puertoDestino) {
	int socketDestino;
	tSocketInfo infoSocketDestino;
	infoSocketDestino.sin_family = AF_INET;
	infoSocketDestino.sin_port = htons(puertoDestino);
	inet_aton(ipDestino, &infoSocketDestino.sin_addr);
	memset(&(infoSocketDestino.sin_zero), '\0', 8);

	if ((socketDestino = crearSocket()) == EXIT_FAILURE) {
		perror("Error al crear socket");
		return EXIT_FAILURE;
	}

	if (connect(socketDestino, (struct sockaddr*) &infoSocketDestino, sizeof(infoSocketDestino)) == -1) {
		perror("Error al conectar socket");
		close(socketDestino);
		return EXIT_FAILURE;
	}

	return socketDestino;
}

int desconectarseDe(int socket) {
	if (close(socket)) {
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}

int32_t enviarMensaje(int32_t numSocket, t_header header, t_contenido mensaje, t_log *logger) {

	if(strlen(mensaje)>sizeof(t_contenido)){
		log_error(logger, "Error en el largo del mensaje, tiene que ser menor o igual al máximo: %s",MSG_SIZE);
		return EXIT_FAILURE;
	}

	t_mensajes *s = malloc(sizeof(t_mensajes));
	s->id = header;
	strcpy(s->contenido, mensaje);
	log_info(logger, "Se ENVIA por SOCKET:%d - HEADER:%s MENSAJE:\"%s\" ",
			numSocket, getDescription(s->id), s->contenido);
	int n = send(numSocket, s, sizeof(*s), 0);
	if(n != STRUCT_SIZE){
		log_error(logger, "#######################################################################");
		log_error(logger, "##    ERROR en el envío de mensajes: no se envió todo. socket: %d    ##", numSocket);
		log_error(logger, "#######################################################################");
	}
	free(s);
	return n;
}

t_header recibirMensaje(int numSocket, t_contenido mensaje, t_log *logger) {

	char buffer[STRUCT_SIZE];

	log_debug(logger, "Estoy en recv");
	int n = recv(numSocket, buffer, STRUCT_SIZE, 0);

	if (n == STRUCT_SIZE) {
		t_mensajes* s = (t_mensajes*)buffer;
		strcpy(mensaje, s->contenido);
		log_debug(logger, "Se RECIBE por SOCKET:%d - HEADER:%s MENSAJE:\"%s\" ",
				numSocket, getDescription(s->id),mensaje);
		return s->id;
	} else {
		if (n == 0) { // Conexión remota cerrada
			log_info(logger, "El socket %d cerró la conexion.", numSocket);
			strcpy(mensaje, "");
			return ERR_CONEXION_CERRADA;
		} else { // El mensaje tiene un tamaño distinto al esperado
			log_error(logger, "#######################################################");
			log_error(logger, "##    ERROR en el recibo de mensaje del socket %d    ##", numSocket);
			log_error(logger, "#######################################################");
			strcpy(mensaje, "");
			//usleep(500000);
			return ERR_ERROR_AL_RECIBIR_MSG;
		}
	}
}

int cerrarSocket(int numSocket, fd_set *fd){

	close(numSocket);

	if (fd!=NULL) {
		FD_CLR(numSocket,fd);
	}
	return EXIT_SUCCESS;
}

char* getDescription(int item){

	switch(item){
	case 	ERR_CONEXION_CERRADA: 	return 	"ERR_CONEXION_CERRADA  "	;
	case	ERR_ERROR_AL_RECIBIR_MSG:	return 	"ERR_ERROR_AL_RECIBIR_MSG "	;
	case	ERR_ERROR_AL_ENVIAR_MSG:	return 	"ERR_ERROR_AL_ENVIAR_MSG "	;
	case	CODE_HAY_MAS_LINEAS:	return 	"CODE_HAY_MAS_LINEAS "	;
	case 	CODE_HAY_MAS_LINEAS_OK: return "CODE_HAY_MAS_LINEAS_OK";
	case	CODE_FIN_LINEAS:	return 	"CODE_FIN_LINEAS "	;
	case	CON_TO_KRN_CODE:	return 	"CON_TO_KRN_CODE "	;
	case	CON_TO_KRN_HANDSHAKE:	return 	"CON_TO_KRN_HANDSHAKE"	;
	case	KERNEL_TO_CPU_HANDSHAKE:	return 	"KERNEL_TO_CPU_HANDSHAKE "	;
	case	KERNEL_TO_CPU_TCB:	return 	"KERNEL_TO_CPU_TCB"	;
	case	KERNEL_TO_CPU_VAR_COMPARTIDA_OK:	return 	"KERNEL_TO_CPU_VAR_COMPARTIDA_OK "	;
	case	KERNEL_TO_CPU_VAR_COMPARTIDA_ERROR:	return 	"KERNEL_TO_CPU_VAR_COMPARTIDA_ERROR "	;
	case	KERNEL_TO_CPU_ASIGNAR_OK:	return 	"KERNEL_TO_CPU_ASIGNAR_OK "	;
	case	KERNEL_TO_CPU_BLOCKED:	return 	"KERNEL_TO_CPU_BLOCKED "	;
	case	KERNEL_TO_CPU_OK:	return 	"KERNEL_TO_CPU_OK "	;
	case	KERNEL_TO_CPU_TCB_QUANTUM:	return 	"KERNEL_TO_CPU_TCB_QUANTUM "	;
	case	KERNEL_TO_PRG_IMPR_PANTALLA:	return 	"KERNEL_TO_PRG_IMPR_PANTALLA "	;
	case	KERNEL_TO_PRG_IMPR_IMPRESORA:	return 	"KERNEL_TO_PRG_IMPR_IMPRESORA "	;
	case	KERNEL_TO_PRG_END_PRG:	return 	"KERNEL_TO_PRG_END_PRG "	;
	case	KERNEL_TO_PRG_NO_MEMORY:	return 	"KERNEL_TO_PRG_NO_MEMORY "	;
	case	KERNEL_TO_MSP_MEM_REQ:	return 	"KRN_TO_MSP_MEM_REQ "	;
	case	KERNEL_TO_MSP_ELIMINAR_SEGMENTOS:	return 	"KERNEL_TO_UMV_ELIMINAR_SEGMENTOS "	;
	case	KERNEL_TO_MSP_HANDSHAKE:	return 	"KERNEL_TO_MSP_HANDSHAKE "	;
	case	KERNEL_TO_MSP_ENVIAR_BYTES:	return 	"KRN_TO_MSP_ENVIAR_BYTES "	;
	case	KERNEL_TO_MSP_ENVIAR_ETIQUETAS:	return 	"KERNEL_TO_MSP_ENVIAR_ETIQUETAS "	;
	case	KERNEL_TO_MSP_SOLICITAR_BYTES:	return 	"KERNEL_TO_MSP_SOLICITAR_BYTES "	;

	case	CPU_TO_MSP_INDICEYLINEA:	return 	"CPU_TO_MSP_INDICEYLINEA "	;
	case	CPU_TO_MSP_HANDSHAKE:	return 	"CPU_TO_MSP_HANDSHAKE "	;
	case	CPU_TO_MSP_CAMBIO_PROCESO:	return 	"CPU_TO_MSP_CAMBIO_PROCESO "	;
	case	CPU_TO_MSP_SOLICITAR_BYTES:	return 	"CPU_TO_MSP_SOLICITAR_BYTES"	;

	case    CPU_TO_MSP_ENVIAR_BYTES: return "CPU_TO_MSP_ENVIAR_BYTES";
	case	CPU_TO_KERNEL_HANDSHAKE:	return 	"CPU_TO_KERNEL_HANDSHAKE "	;
	case	CPU_TO_KERNEL_NEW_CPU_CONNECTED:	return 	"CPU_TO_KERNEL_NEW_CPU_CONNECTED "	;
	case	CPU_TO_KERNEL_END_PROC:	return 	"CPU_TO_KRN_END_PROC "	;
	case	CPU_TO_KERNEL_FINALIZO_QUANTUM_NORMALMENTE:	return 	"CPU_TO_KERNEL_END_PROC_QUANTUM "	;
	case	CPU_TO_KERNEL_END_PROC_QUANTUM_SIGNAL: return "CPU_TO_KERNEL_END_PROC_QUANTUM_SIGNAL";
	case 	CPU_TO_KERNEL_END_PROC_ERROR:		return "CPU_TO_KERNEL_END_PROC_ERROR";
	case	CPU_TO_KERNEL_IMPRIMIR:	return 	"CPU_TO_KERNEL_IMPRIMIR "	;
	case	CPU_TO_KERNEL_IMPRIMIR_TEXTO:	return 	"CPU_TO_KERNEL_IMPRIMIR_TEXTO "	;
	case	CPU_TO_KERNEL_OK:	return 	"CPU_TO_KERNEL_OK "	;
	case	MSP_TO_KERNEL_MEMORY_OVERLOAD:	return 	"MSP_TO_KERNEL_MEMORY_OVERLOAD "	;
	case	MSP_TO_KERNEL_SEGMENTO_CREADO:	return 	"MSP_TO_KERNEL_SEGMENTO_CREADO "	;
	case	MSP_TO_KERNEL_ENVIO_BYTES:	return 	"MSP_TO_KERNEL_ENVIO_BYTES "	;
	case	MSP_TO_KERNEL_HANDSHAKE_OK:	return 	"MSP_TO_KERNEL_HANDSHAKE_OK "	;
	case	MSP_TO_CPU_BYTES_ENVIADOS:	return 	"MSP_TO_CPU_BYTES_ENVIADOS "	;
	case	MSP_TO_CPU_BYTES_RECIBIDOS:	return 	"MSP_TO_CPU_BYTES_RECIBIDOS "	;
	case	MSP_TO_CPU_SENTENCE:	return 	"MSP_TO_CPU_SENTENCE "	;
	case	MSP_TO_CPU_SEGM_FAULT: 	return 	"MSP_TO_CPU_SEGM_FAULT  "	;
	case	SYSCALL_WAIT_REQUEST: 	return 	"SYSCALL_WAIT_REQUEST  "	;
	case	SYSCALL_SIGNAL_REQUEST: 	return 	"SYSCALL_SIGNAL_REQUEST  "	;

		default:  return "---DEFAULT--- (mensaje sin definir)";
	}
	return "";

}


// **** FUNCIONES AUXILIARES ****
char* separarLineas(char* linea){
	int i;
	for(i=0; linea[i] != '\0'; i++){
		if( linea[i] == '\n' ){
			linea[i] = '~';
		}
	}
	return linea;
}

/**
 * @NAME: imprimir_en_impresora
 * @DESC: Imprime en impresora (Solicitado por el Kernel)
 * @Parameters:
 * 		numSocket = socket destinatario;
 *
 * 		authorizedHeader = el header que confirma la recepcion de código! ***Tiene que ser el mismo header que usa
 * 							quien envia el mensaje. (validación)***
 *
 * 		stringCode = código a recibir, por referencia para recibir el contenido!;
 * 		logger = log de quien consume esta funcionalidad.
 */
char* recibirCodigo(int32_t numSocket, t_header authorizedHeader, t_log *logger) {

	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));
	char* stringCode = string_new();

	strcpy(mensaje, "Operacion de recepción de código AnSISOP autorizada!...");
	enviarMensaje(numSocket, authorizedHeader, mensaje, logger);

	memset(mensaje, 0, sizeof(t_contenido));
	t_header response = recibirMensaje(numSocket, mensaje, logger);
	enviarMensaje(numSocket, CODE_HAY_MAS_LINEAS_OK, "", logger);

	while(response != CODE_FIN_LINEAS){
		string_append(&stringCode, mensaje);
		string_append(&stringCode, "\n");
		memset(mensaje, 0, sizeof(t_contenido));
		response = recibirMensaje(numSocket, mensaje, logger);
		enviarMensaje(numSocket, CODE_HAY_MAS_LINEAS_OK, "", logger);
	}

	return stringCode;
}

/**
 * @NAME: imprimir_en_impresora
 * @DESC: Imprime en impresora (Solicitado por el Kernel)
 * @Parameters:
 * 		numSocket = socket destinatario;
 * 		header = el header de cada enviarMensaje presente en la iteracion;
 * 		initialMessage = acá podes poner algún dato que quieras agregar al momento de avisarle al
* 						 destinatario, que le estas por enviar código.
 * 		stringCode = código a enviar;
 * 		logger = log de quien consume esta funcionalidad.
 */
int32_t enviarCodigo(int32_t numSocket, t_header header, t_contenido initialMessage, char* stringCode, t_log *logger, bool NoBrNoComment) {

	t_contenido mensaje;
	
	/*"Aviso al kernel que le voy a pasar el codigo AnSISOP y espero el ok para comenzar transaccion!"*/
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, initialMessage);
	
	enviarMensaje(numSocket, header, mensaje, logger);
	t_header kernelOkResponse = -1;
	
	while(kernelOkResponse != header)
		kernelOkResponse = recibirMensaje(numSocket, mensaje, logger);

	//Me reemplaza los \n por \0 para poder utilizarlo como token e iterar envios.
	separarLineas(stringCode);

	char* buffer;		//Guardo el texto a medida que voy leyendo
	int32_t position = 0;	//Posicion
	int32_t offset = 0;		//Desplazamiento dentro de mi buffer (seek)

	int32_t tamanioTotal = strlen(stringCode);
	char* test = string_new();

	while( position<tamanioTotal ){

		buffer = stringCode+position;

		if(buffer[offset] == '~') {
			memset(mensaje, 0, sizeof(t_contenido));
			strcpy(mensaje, string_substring(stringCode, position, offset));

			test = string_duplicate(mensaje);
			string_trim(&test);

			if(!NoBrNoComment || (!string_equals_ignore_case(string_substring(test, 0, 1), "#") && !string_equals_ignore_case(string_substring(test, 0, 1), ""))){
				enviarMensaje(numSocket, CODE_HAY_MAS_LINEAS, mensaje, logger);
				recibirMensaje(numSocket, mensaje, logger);
			}

			position += (offset + 1);
			offset = -1;
		}
		offset += 1 ;
	}

	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, "Code Message transaction completed without errors! :)");
	enviarMensaje(numSocket, CODE_FIN_LINEAS, mensaje, logger);


	return 1;
}
