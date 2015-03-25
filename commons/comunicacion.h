//include comunicacion.h
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <time.h>
#include <netinet/tcp.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>

#define ERROR		    0
#define EXITO		    1
#define WARNING         2

#define OTRO			9

#pragma pack(1)
typedef struct header_s
{
  char id[16];
  char tipo;
  int  largo_mensaje;

}header_t;
#pragma pack(0)

header_t* crearHeader();
void initHeader(header_t* header);
int enviar(int sock, char *buffer, int tamano);
int recibir(int sock, char *buffer, int tamano);
int conectar(char ip[15+1], int puerto, int *sock);
int aceptar_conexion(int *listener, int *nuevo_sock);

int enviar_header (int sock, header_t *header);
int recibir_header(int sock, header_t *header, fd_set *master/*por si se desconecta*/, int *seDesconecto);
int recibir_header_simple(int sock, header_t *header);
int aceptar_conexion(int *listener, int *nuevoSock);
int agregar_descriptor(int desc, fd_set *listaDesc, int *maxDesc);
int quitar_descriptor(int desc, fd_set *listaDesc, int *maxDesc);
int crear_listener(int puerto, int *listener);
void genId(char idMsg[]);
