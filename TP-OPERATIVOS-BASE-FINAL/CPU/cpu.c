/*
 ============================================================================
 Name        : cpu.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "cpu.h"

int main() {



	//	unsigned int mi_pid;

	//	mi_pid = process_getpid();

	//	log_info(logs,"el valor del pid de mi cpu es: %d,",mi_pid);

		A = 0;
		B = 0;
		D = 0;
		C = 0;
		E = 0;

		logs = log_create("log", "CPU.c", 1, LOG_LEVEL_TRACE);

		log_info(logs,"Los registros inicialmente valen A:%d B:%d C:%d D:%d E:%d",A,B,C,D,E);



		iniciarPrograma(); // Levanto el archivo de configuracion, en caso de que nose pueda tiro error
		ipKernel   = config_get_string_value(config, IP_KERNEL);
		char* ipMSP      = config_get_string_value(config, IP_MSP);
		puertoKernel  = config_get_int_value(config,PUERTO_KERNEL);
		int32_t puertoMSP     = config_get_int_value(config, PUERTO_MSP);

			// signals
				signal(SIGINT,manejar_senial);
				signal(SIGTERM,manejar_senial);
				signal(SIGUSR1,manejar_senial);


		// me conecto al kernel y MSP
		log_info(logs, "Conexion a kernel ip:%s y puerto:%d", ipKernel, puertoKernel);
		log_info(logs, "Conexion a MSP ip:%s y puerto:%d", ipMSP, puertoMSP);

		//conexion al Kernel
	//	conexion_Kernel();

		//conexion a MSP
			socketMSP = conectarAServidor(ipMSP, puertoMSP);
				while (socketMSP == EXIT_FAILURE) {
						log_info(logs,
						"Despierten a la MSP! Se reintenta conexion en unos segundos \n");
						sleep(10);
						socketMSP = conectarAServidor(ipMSP, puertoMSP);
				}

			socketKernel = conectarAServidor(ipKernel,7000);

			while (socketKernel == EXIT_FAILURE) {
						log_info(logs,
						"Despierten a la KERNEL! Se reintenta conexion en unos segundos \n");
						sleep(10);
						socketKernel = conectarAServidor(ipKernel,7000);
				}


		//Hago el handshake con la MSP

		t_contenido mensaje;
		memset(mensaje,0,sizeof(t_contenido));
		strcpy(mensaje,"hola");

		enviarMensaje(socketMSP,CPU_TO_MSP_HANDSHAKE,mensaje,logs);

		enviarMensaje(socketKernel,CPU_TO_KERNEL_HANDSHAKE,mensaje,logs);

		log_info(logs,"el newfd vale %d",newFD);

		int seguir = 1;

		while(seguir == 1){

			// Aca deberia quedarme a la escucha de que el Kernel me me mande el TCB

			t_contenido mensaje_para_recibir_TCB;
			memset(mensaje_para_recibir_TCB,0,sizeof(t_contenido));
			recibirMensaje(socketKernel,mensaje_para_recibir_TCB,logs);

			char** array = string_get_string_as_array(mensaje_para_recibir_TCB);

			TCB = malloc(sizeof(registro_TCB));

			TCB->pid = atoi(array[0]); //1234;
			TCB->base_segmento_codigo = atoi(array[1]);//1048576;
			TCB->tamanio_segmento_codigo = atoi(array[2]);
			QUANTUM = 18;
		//	TCB->QUANTUM = atoi(array[3]);
			TCB->indicador_modo_kernel = atoi(array[4]);
			TCB->base_stack = atoi(array[5]);
			TCB->cursor_stack = atoi(array[6]);

			M = TCB->base_segmento_codigo;
			P = TCB->puntero_instruccion;
			X = TCB->base_stack;
			S = TCB->cursor_stack;

			log_info(logs,"el valor del pid es %d",TCB->pid);
			log_info(logs,"el valor de la base es %d",TCB->base_segmento_codigo);
			log_info(logs,"el valor del quantum  es %d",QUANTUM);
			log_info(logs,"el valor del modo es %d",TCB->indicador_modo_kernel);
			log_info(logs,"el valor de la base del stack es %d",TCB->base_stack);
			log_info(logs,"el valor de la base del stack es %d",TCB->cursor_stack);


			program_counter = TCB->base_segmento_codigo;
			MODO = TCB->indicador_modo_kernel;

			log_info(logs,"el modo es %d",MODO);

			systemCall = 0;
			cont = 0;
			termino_proceso_XXXX = 0;

			while(cont < QUANTUM && MODO == MODO_USUARIO){

				t_contenido mensaje_para_solicitar_bytes_MSP;
				memset(mensaje_para_solicitar_bytes_MSP,0,sizeof(t_contenido));
				strcpy(mensaje_para_solicitar_bytes_MSP,string_from_format("[%d,%d,%d]",TCB->pid,program_counter,sizeof(int32_t)));
				enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_solicitar_bytes_MSP,logs);

				usleep(60000);

				//Recibo Instruccion
				t_contenido mensaje_para_recibir_direccion;
				t_header header_verificar_mensaje = recibirMensaje(socketMSP,mensaje_para_recibir_direccion,logs);

				//Verifico que sea una instruccion

				if(header_verificar_mensaje == MSP_TO_CPU_BYTES_ENVIADOS){
					log_info(logs,"La instruccion es %s",mensaje_para_recibir_direccion);
					buscador_de_instruccion(mensaje_para_recibir_direccion);
					//sseguir = 0; //Es para salir el por el momento
				}

				cont ++;

				log_info(logs,"el contador es %d",cont);

			}// Fin while cont

		while(TCB->indicador_modo_kernel == MODO_KERNEL && systemCall == 1 && termino_proceso_XXXX == 1){ //Para salir modifico en la instruccion XXXX el systemCall = 0

				//Pido la instruccion a la MSP
				t_contenido mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP;
				memset(mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,0,sizeof(t_contenido));
				strcpy(mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP, string_from_format("[%d,%d,%d]",TCB->pid,TCB->puntero_instruccion,sizeof(int32_t)));
				enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,logs);

				t_header header_verificar_mensaje = recibirMensaje(socketMSP,mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,logs);

				if(header_verificar_mensaje == MSP_TO_CPU_BYTES_ENVIADOS){
					log_info(logs,"La instruccion es %s",mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP);
					buscador_de_instruccion(mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP);
					seguir = 0; //Es para salir el por el momento
				}

			}//Fin while(systemCall)


			if(cont == QUANTUM ){

				t_contenido mensaje_para_el_kernel;
				memset(mensaje_para_el_kernel,0,sizeof(t_contenido));
				strcpy(mensaje_para_el_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,program_counter,TCB->indicador_modo_kernel,A,B,C,D,E));
				enviarMensaje(socketKernel,CPU_TO_KERNEL_FINALIZO_QUANTUM_NORMALMENTE,mensaje_para_el_kernel,logs);

				}

			free(TCB);

			log_info(logs,"el contador final  es %d",cont);

		}//Fin while seguir

		return EXIT_SUCCESS;
}

void manejar_senial(int senial){

	switch(senial){

	case SIGTERM:
	case SIGUSR1:
	case SIGINT:

		if(!ejecutando){
						log_info(logs, "Se recibio la senial y nadie estaba corriendo por aca, cierro el CPU [FIN]");
						exit(0);
					}
					else if(!seguir) {
						log_info(logs, "Se recibio SIGINT por SEGUNDA vez, cierro el CPU [FIN]");
						exit(0);
					}
					else{
						log_info(logs, "Se recibio SIGINT, termino el CPU despues de concluir ejecucion de quantum");
						seguir = false;
					}
					break;
				default:
					log_warning(logs, "Se recibio una señal no manejada..");
					break;
			}
		}

uint32_t generarDireccionLogica(int numeroSegmento, int numeroPagina, int offset)
{
	uint32_t direccion = numeroSegmento;
	direccion = direccion << 12;
	direccion = direccion | numeroPagina;
	direccion = direccion << 8;
	direccion = direccion | offset;

	return direccion;
}


void obtenerUbicacionLogica(uint32_t direccion, int *numeroSegmento, int *numeroPagina, int *offset)
{
	*offset = direccion & 0xFF;
	*numeroPagina = (direccion >> 8) & 0xFFF;
	*numeroSegmento = (direccion >> 20) & 0xFFF;
}


uint32_t aumentarProgramCounter(uint32_t programCounterAnterior, int bytesASumar)
{
	uint32_t nuevoProgramCounter;
	int numeroSegmento, numeroPagina, offset;

	obtenerUbicacionLogica(programCounterAnterior, &numeroSegmento, &numeroPagina, &offset);

	if (offset + bytesASumar >= TAMANIO_PAGINA)
	{
		int faltaParaCompletarPagina = TAMANIO_PAGINA - offset ;
		int quedaParaSumar = bytesASumar - faltaParaCompletarPagina ;
		int paginaFinal, offsetPaginaFinal;

		paginaFinal = (numeroPagina + quedaParaSumar / TAMANIO_PAGINA) + 1;
		offsetPaginaFinal = quedaParaSumar % TAMANIO_PAGINA;

		nuevoProgramCounter = generarDireccionLogica(numeroSegmento, paginaFinal, offsetPaginaFinal);


	}

	else
	{
		nuevoProgramCounter = generarDireccionLogica(numeroSegmento, numeroPagina, offset + bytesASumar);
	}


	return nuevoProgramCounter;
}



void conexion_Kernel(){

	struct sockaddr_in my_addr, their_addr;

	int socketFD;

	socketFD = crearSocket();

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(puertoKernel);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), '\0', sizeof(struct sockaddr_in));

	bindearSocket(socketFD, my_addr);

	escucharEn(socketFD);



		socklen_t sin_size;

				sin_size = sizeof(struct sockaddr_in);


				log_trace(logs, "A la espera de nuevas conexiones");

				if((newFD = accept(socketFD,(struct sockaddr *)&their_addr, &sin_size)) == -1)
				{
					perror("accept");

				}

				log_trace(logs, "Recibí conexion de %s", inet_ntoa(their_addr.sin_addr));

				t_contenido mensajeParaRecibirConexionCpu;
				memset(mensajeParaRecibirConexionCpu, 0, sizeof(t_contenido));

				t_header header_conexion_kernel = recibirMensaje(newFD, mensajeParaRecibirConexionCpu, logs);


				if(header_conexion_kernel == KERNEL_TO_CPU_HANDSHAKE){
					log_info(logs,"El HandShake se hace con exito");
				}

	}





