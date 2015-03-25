
#ifndef UMV_H_
#define UMV_H_

#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>

#include <Commons/commons/collections/list.h>
#include <Commons/commons/config.h>
#include <Commons/commons/log.h>
#include <Commons/commons/sockets.h>

#define integer int32_t
#define BACKLOGUMV 10 // Cuántas conexiones pendientes se mantienen en cola UMV
#define BASE_INEXISTENTE  -1
#define STACK_OVERFLOW    -2

#define STR_BASE_INEXISTENTE "BASE_INEXISTENTE"
#define STR_STACK_OVERFLOW "STACK_OVERFLOW"

#define ALGFF 0
#define ALGWF 1

// DATOS A USAR
typedef char* t_memoria;

// estructura que define un segmento de cualquier tipo
typedef struct _t_segmento {

	bool libre;         // si esta libre o no
	integer pid;        // programa al que pertenece
	integer cpuId;      // ultimo CPU que la accedió (se inicializa en valor invalido)
	integer base;       // direccion virtual de inicio del segmento
	integer inicio;     // direccion real de inicio del segmento
	integer tamanio;    // tamaño total del segmento

} t_segmento;

static void liberar(t_segmento *self);
char* getBytes(integer base, integer offset, integer tamanio);
integer writeBuffer(integer base, integer offset, integer tamanio, char* buffer);
integer _getDirectionByBase(integer base);
integer _getSizeByBase(integer base);
bool accessAllowed(integer cpuId, integer base);
void consoleListener() ;
void setAlgoritmo(integer alg);
void printError(char* str1);
void printComando(char* str1);
void Dump();
void Compactar();
integer _subirSegmento(t_segmento* segmento, integer inicioSegmLibre);
void testCompactar2();
void testCompactarWF();
void testCompactar();
void* listenCPUandKERNEL(void* dato);
void* newCpu(void* socketCpu);
void cambioDeProceso(pid, cpuId);
void* newKernel(void* socketK);
char** split_setBytes(char* text) ;
integer getRandomBase();
integer crearSegmento(integer pid, integer tamanio);
bool strchk(char *s);
void eliminarSegmento(integer pid);
void setRetardo(integer nuevoTiempo);
void esperar();
void sigchld_handler(int s);
void showMemory();
void Play();
char** get_string_as_array(char* text);


#endif //UMV_H_
