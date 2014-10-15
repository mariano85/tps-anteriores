
#ifndef SOCKETS_H
#define SOCKETS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <unistd.h>

/*Commons includes*/
#include <string.h>
#include  <commons/log.h>
#include <commons/string.h>

#define MSG_SIZE 256
#define STRUCT_SIZE (MSG_SIZE + sizeof(t_header))
#define BACKLOG 10 // cantidad máxima de sockets a mantener en pool de espera

typedef struct sockaddr_in tSocketInfo;
typedef struct sockaddr_in t_socket_info;

//COLORS
#define none    "\033[0m"    /* to flush the previous property */
#define red     "\033[1;31m" /* 0 -> normal ;  31 -> red */
#define cyan    "\033[1;36m" /* 1 -> bold ;  36 -> cyan */
#define green   "\033[1;32m" /* 4 -> underline ;  32 -> green */
#define blue    "\033[1;34m" /* 9 -> strike ;  34 -> blue */
#define black   "\033[0;30m"
#define brown   "\033[0;33m"
#define gray    "\033[0;37m"
#define magenta "\033[0;35m"

// Ejemplos:
// printf("%sHello, %sworld!%s\n", red, blue, none);
// printf("%sHello%s, %sworld!\n", green, none, cyan);
// printf("%s", none);


char* getDescription(int item);

/**
 * Nomenclatura de los mensajes:
 * 'PROC1_TO_PROC2_TIPO_DE_MENSAJE' // Params: 'A', 'B', 'C'
 *  y el proceso que lo reciba deberá esperar los parámetros 'A', 'B', 'C'
 */
typedef enum {

	/*Errores de conexion */
	ERR_CONEXION_CERRADA,      // evaluar solo este mensaje por si se cierra una conexion
	ERR_ERROR_AL_RECIBIR_MSG,
	ERR_ERROR_AL_ENVIAR_MSG,

	/*para envio de codigo entre procesos*/
	CODE_HAY_MAS_LINEAS,
	CODE_HAY_MAS_LINEAS_OK,
	CODE_FIN_LINEAS,

	/*Enviados desde el kernel*/
	KRN_TO_CPU_HANDSHAKE,
	KRN_TO_CPU_TCB,
	KRN_TO_CPU_VAR_COMPARTIDA_OK,
	KRN_TO_CPU_VAR_COMPARTIDA_ERROR,
	KRN_TO_CPU_ASIGNAR_OK,
	KRN_TO_CPU_BLOCKED,
	KRN_TO_CPU_OK,
	KRN_TO_CPU_PCB_QUANTUM,

	KRN_TO_PRG_IMPR_PANTALLA,  // imprimir en pantalla
	KRN_TO_PRG_IMPR_IMPRESORA, // imprimir en impresora
	KRN_TO_PRG_IMPR_VARIABLES, // imprimir valores finales de variables
	KRN_TO_PRG_END_PRG,
	KRN_TO_PRG_NO_MEMORY,

	KRN_TO_MSP_MEM_REQ,
	KRN_TO_MSP_ELIMINAR_SEGMENTOS,
	KRN_TO_MSP_HANDHAKE,
	KRN_TO_MSP_ENVIAR_BYTES,
	KRN_TO_MSP_ENVIAR_ETIQUETAS,
	KRN_TO_MSP_SOLICITAR_BYTES,

	/*Enviados desde el programa*/
	PRG_TO_KRN_HANDSHAKE,
	PRG_TO_KRN_CODE,		   // codigo Ansisop
	PRG_TO_KRN_OK,

	/*Enviados desde el cpu*/
	CPU_TO_MSP_INDICEYLINEA,
	CPU_TO_MSP_HANDSHAKE,
	CPU_TO_MSP_CAMBIO_PROCESO,
	CPU_TO_MSP_SOLICITAR_BYTES,
	CPU_TO_MSP_ENVIAR_BYTES,
	CPU_TO_MSP_SOLICITAR_ETIQUETAS,


	CPU_TO_KRN_HANDSHAKE,
	CPU_TO_KRN_NEW_CPU_CONNECTED,
	CPU_TO_KRN_END_PROC,
	CPU_TO_KRN_END_PROC_ERROR,
	CPU_TO_KRN_END_PROC_QUANTUM,
	CPU_TO_KRN_END_PROC_QUANTUM_SIGNAL,
	CPU_TO_KRN_IMPRIMIR,
	CPU_TO_KRN_IMPRIMIR_TEXTO,
	CPU_TO_KRN_OK,

	/*Enviados desde el umv*/
	UMV_TO_KRN_MEMORY_OVERLOAD, // No existe espacio suficiente para crear un segmento
	UMV_TO_KRN_SEGMENTO_CREADO,
	UMV_TO_KRN_ENVIO_BYTES,
	UMV_TO_KRN_HANDSHAKE_OK,
	UMV_TO_CPU_BYTES_ENVIADOS,
	UMV_TO_CPU_BYTES_RECIBIDOS,

	UMV_TO_CPU_SENTENCE,
	UMV_TO_CPU_SEGM_FAULT,     // El acceso a memoria esta por fuera de los rangos permitidos.

	/*Para comunicacion entre CPU -> KERNEL SYSTEM CALLS*/
	SYSCALL_IO_REQUEST,
	SYSCALL_GET_REQUEST,
	SYSCALL_SET_REQUEST,
	SYSCALL_WAIT_REQUEST,
	SYSCALL_SIGNAL_REQUEST,

	FIN

} t_header;

/* Defino los tipos de señales que se pueden mandar
 *
 * Sintaxis correcta para escribir una nueva señal:
 *
 * 	#define [Origen]_TO_[Destino]_[Mensaje] numeroIncremental
 *
 * 	Donde:
 * 		[Origen] es quien manda el mensaje
 * 		[Destino] es quien recive el mensaje
 * 		[Mensaje] lo que se le quiere mandar
 * 		numeroIncrementar un numero mas que la señal anterior
 */

typedef char t_contenido[MSG_SIZE];

typedef struct ts_mensajes {
	t_header id;
	t_contenido contenido;
}t_mensajes;


/*
 * crearSocket: Crea un nuevo socket.
 * @return: El socket creado.
 * 			EXIT_FAILURE si este no pudo ser creado.
 */
int crearSocket();


/*
 * bindearSocket: Vincula un socket con una direccion de red.
 *
 * @arg socket:		Socket a ser vinculado.
 * @arg socketInfo: Protocolo, direccion y puerto de escucha del socket.
 * @return:			0 si el socket ha sido vinculado correctamente.
 * 					EXIT_FAILURE si se genera error.
 */
int bindearSocket(int unSocket, tSocketInfo socketInfo);


/*
 * escucharEn: Define el socket de escucha.
 *
 * @arg socket:		Socket de escucha.
 * @return:			EXIT_SUCCESS si la conexion es exitosa.
 * 					EXIT_FAILURE si se genera error.
 */
int escucharEn(int unSocket);


/*
 * conectarServidor: Crea una conexion a un servidor como cliente.
 *
 * @arg puertoDestino: Puerto de escucha del servidor.
 * @arg ipDestino: 	   IP del servidor.
 * @return: 		   int: Socket de conexión con el server.
 *					   EXIT_FAILURE: No se pudo conectar.
 */
int conectarAServidor(char *ipDestino, unsigned short puertoDestino);


/*
 * desconectarseDe: Se desconecta el socket especificado.
 *
 * @arg socket: Socket al que estamos conectados.
 * @return:		EXIT_SUCCESS: La desconexion fue exitosa.
 * 				EXIT_FAILURE: Se genero error al desconectarse.
 */
int desconectarseDe(int socket);

/*
 * Funcion enviar mensaje, recibe como parámetros:
 * socket, el header y el mensaje propiamente dicho
 * devuelve el numero de bytes enviados
 */
int enviarMensaje(int numSocket, t_header header, t_contenido mensaje, t_log *logger);

/*lo mismo pero para codigo*/
int32_t enviarCodigo(int32_t numSocket, t_header header, t_contenido initialMessage, char* stringCode, t_log *logger, bool NoBrNoComment);

/*
 * Funcion recibir mensaje, recive como parámetros:
 * socket y mensaje donde se pondŕa en mensaje,
 * y devuelve el header, para evaluar la accion a tomar.
 */
t_header recibirMensaje(int numSocket, t_contenido mensaje, t_log *logger);

/*lo mismo pero para codigo*/
char* recibirCodigo(int numSocket, t_header authorizedHeader, t_log *logger);

//Funciones auxiliares
char* separarLineas(char* codigo);

/*
 * Funcion cerrar un socket, y eliminarlo de un fd_set[opcional]:
 * retorna EXIT_SUCCESS si se realizó bien, EXIT_FAILURE sino.t_header recibirCodigo(int numSocket, t_header authorizedHeader, char** stringCode, t_log *logger)
 * ej: cerrarSocket(mySocket, &read_fds);t_header recibirCodigo(int numSocket, t_header authorizedHeader, char**t_header recibirCodigo(int numSocket, t_header authorizedHeader, char** stringCode, t_log *logger) stringCode, t_log *logger)
 */
int cerrarSocket(int numSocket, fd_set* fd);

#endif
