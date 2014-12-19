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

	seguir = true;
	someoneRunning = false;
	murioKernel = false;

	//	unsigned int mi_pid;

	//	mi_pid = process_getpid();

	//	log_info(logs,"el valor del pid de mi cpu es: %d,",mi_pid);

		A = 0;
		B = 0;
		D = 0;
		C = 0;
		E = 0;

		FILE *log = fopen("log","w");
		fflush(log);
		fclose(log);

		logs = log_create("log", "CPU.c", 1, LOG_LEVEL_TRACE);

		log_info(logs,"Los registros inicialmente valen A:%d B:%d C:%d D:%d E:%d",A,B,C,D,E);



		iniciarPrograma(); // Levanto el archivo de configuracion, en caso de que nose pueda tiro error
		ipKernel   = config_get_string_value(config, IP_KERNEL);
		char* ipMSP      = config_get_string_value(config, IP_MSP);
		puertoKernel  = config_get_int_value(config,PUERTO_KERNEL);
		int32_t puertoMSP     = config_get_int_value(config, PUERTO_MSP);
		int32_t retardo     = config_get_int_value(config, RETARDO);

		// signals
		signal(SIGINT, rutinasSeniales);



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
						sleep(5);
						socketMSP = conectarAServidor(ipMSP, puertoMSP);
				}

			socketKernel = conectarAServidor(ipKernel,7000);

			while (socketKernel == EXIT_FAILURE) {
						log_info(logs,
						"Despierten al KERNEL! Se reintenta conexion en unos segundos \n");
						sleep(5);
						socketKernel = conectarAServidor(ipKernel,7000);
				}


		//Hago el handshake con la MSP

		t_contenido mensaje;
		memset(mensaje,0,sizeof(t_contenido));
		strcpy(mensaje,"hola");

		enviarMensaje(socketMSP,CPU_TO_MSP_HANDSHAKE,mensaje,logs);

		enviarMensaje(socketKernel,CPU_TO_KERNEL_HANDSHAKE,mensaje,logs);
		recibirMensaje(socketKernel, mensaje, logs);
		char** array = string_get_string_as_array(mensaje);
		TAMANIO_STACK = atoi(array[1]);
		QUANTUM = atoi(array[0]);

		log_info(logs,"el valor del quantum  es %d",QUANTUM);
		log_info(logs,"el valor del retardo  es %d",retardo);

		int seguir = 1;

		while(seguir == 1){

			flagInterrupcion = false;

			// Aca deberia quedarme a la escucha de que el Kernel me me mande el TCB

			t_contenido mensaje_para_recibir_TCB;
			memset(mensaje_para_recibir_TCB,0,sizeof(t_contenido));
			t_header header =  recibirMensaje(socketKernel,mensaje_para_recibir_TCB,logs);

			if(header == ERR_CONEXION_CERRADA){
						log_info(logs, "Se cerró la conexion con kernel");
						return EXIT_SUCCESS;
					}

			someoneRunning = true;

			char** array = string_get_string_as_array(mensaje_para_recibir_TCB);

			TCB = malloc(sizeof(registro_TCB));

			TCB->pid = atoi(array[0]); //1234;
			TCB->tid = atoi(array[1]);
			TCB->indicador_modo_kernel = atoi(array[2]);
			TCB->base_segmento_codigo = atoi(array[3]);//1048576;
			TCB->tamanio_segmento_codigo = atoi(array[4]);
			TCB->puntero_instruccion = atoi(array[5]);
			TCB->base_stack = atoi(array[6]);
			TCB->cursor_stack = atoi(array[7]);

			A = atoi(array[8]);
			B = atoi(array[9]);
			C = atoi(array[10]);
 			D =	atoi(array[11]);
			E =	atoi(array[12]);

			M = TCB->base_segmento_codigo;
			P = TCB->puntero_instruccion;
			X = TCB->base_stack;
			S = TCB->cursor_stack;

			MODO = TCB->indicador_modo_kernel;

			log_info(logs,"el valor del pid es %d",TCB->pid);
			log_info(logs,"el valor de la base es %d",TCB->base_segmento_codigo);
			log_info(logs,"el modo es %d",MODO);

			cont = 0;
			termino_proceso_XXXX = 1;
			aux_INTE = 0;

			while(cont < QUANTUM && MODO == MODO_USUARIO && aux_INTE == 0 && flagError == false){

				t_contenido mensaje_para_solicitar_bytes_MSP;
				memset(mensaje_para_solicitar_bytes_MSP,0,sizeof(t_contenido));
				strcpy(mensaje_para_solicitar_bytes_MSP,string_from_format("[%d,%d,%d]",TCB->pid,P,sizeof(int32_t)));
				enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_solicitar_bytes_MSP,logs);

				//Recibo Instruccion
				t_contenido mensaje_para_recibir_direccion;
				memset(mensaje_para_recibir_direccion,0,sizeof(t_contenido));
				t_header header_verificar_mensaje = recibirMensaje(socketMSP,mensaje_para_recibir_direccion,logs);

				//Verifico que sea una instruccion

				if(header_verificar_mensaje == MSP_TO_CPU_BYTES_ENVIADOS){
					log_info(logs,"La instruccion es %s",mensaje_para_recibir_direccion);
					buscador_de_instruccion(mensaje_para_recibir_direccion);


				}else{

					verificar_error_de_la_MSP(header_verificar_mensaje);

				}

				cont ++;

				log_info(logs,"el contador es %d",cont);
				usleep(retardo * 1000);

			}// Fin while cont

			someoneRunning = false;

		while(TCB->indicador_modo_kernel == MODO_KERNEL && termino_proceso_XXXX == 1 && flagError == false){ //Para salir modifico en la instruccion XXXX el systemCall = 0

				//Pido la instruccion a la MSP
				t_contenido mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP;
				memset(mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,0,sizeof(t_contenido));
				strcpy(mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP, string_from_format("[%d,%d,%d]",-1,P,sizeof(int32_t)));
				enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,logs);

				t_header header_verificar_mensaje = recibirMensaje(socketMSP,mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,logs);

				if(header_verificar_mensaje == MSP_TO_CPU_BYTES_ENVIADOS){
					log_info(logs,"La instruccion es %s",mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP);
					buscador_de_instruccion(mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP);
				}else{


					verificar_error_de_la_MSP(header_verificar_mensaje);

				}

				cont = 0;

			}//Fin while(systemCall)

			someoneRunning = false;

			if(cont == QUANTUM && flagInterrupcion == false){

				t_contenido mensaje_para_el_kernel;
				memset(mensaje_para_el_kernel,0,sizeof(t_contenido));
				strcpy(mensaje_para_el_kernel, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,M,TCB->tamanio_segmento_codigo,P,X,S,A,B,C,D,E));
				enviarMensaje(socketKernel,CPU_TO_KERNEL_FINALIZO_QUANTUM_NORMALMENTE,mensaje_para_el_kernel,logs);


				}


			free(TCB);

			log_info(logs,"el contador final  es %d",cont);
			usleep(retardo * 1000);

		}//Fin while seguir

		return EXIT_SUCCESS;
}

/*
 * Rutinas de las señales.
 */
void rutinasSeniales(int senial){
	switch (senial) {
		case SIGINT:
		{
		// Interrupcion de programa: Control + c. Ejecutando 2 veces, mata el Cpu
			if(!someoneRunning){
				log_info(logs, "Se recibio la senial y nadie estaba corriendo por aca, cierro el CPU [FIN]");
				exit(0);
			}
			else if(!seguir) {
				log_info(logs, "Se recibio SIGINT por SEGUNDA vez, cierro el CPU [FIN]");
				exit(0);
			}
			else {
				log_info(logs, "Se recibio SIGINT, termino el CPU despues de concluir ejecucion de quantum");
				seguir = false;
			}



			break;
		}
		default:
		{
			log_warning(logs, "Se recibio una señal no manejada..");
			break;
		}
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


void verificar_error_de_la_MSP(int32_t header_de_la_MSP){

	t_contenido mensaje_para_el_kernel;
	memset(mensaje_para_el_kernel,0,sizeof(t_contenido));
	strcpy(mensaje_para_el_kernel,string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_segmento_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack,A,B,C,D,E));

	switch(header_de_la_MSP){

		case MSP_TO_CPU_VIOLACION_DE_SEGMENTO  :
		{
			flagError = true;
			enviarMensaje(socketKernel,MSP_TO_CPU_VIOLACION_DE_SEGMENTO,mensaje_para_el_kernel,logs);
			log_info(logs,"CPU se desconecto por que hubo violaciion de segmento");


		}break;
		case MSP_TO_CPU_PID_INVALIDO :{

			flagError = true;
			enviarMensaje(socketKernel,MSP_TO_CPU_PID_INVALIDO,mensaje_para_el_kernel,logs);
			log_info(logs,"CPU se desconecto por que el PID es invalido");

		}break;
		case MSP_TO_CPU_DIRECCION_INVALIDA :{

			flagError = true;
			enviarMensaje(socketKernel,MSP_TO_CPU_DIRECCION_INVALIDA,mensaje_para_el_kernel,logs);
			log_info(logs,"CPU se desconecto por que la direccion es invalida");

		}break;
		case MSP_TO_CPU_MEMORIA_INSUFICIENTE :{

			flagError = true;
			enviarMensaje(socketKernel,MSP_TO_CPU_MEMORIA_INSUFICIENTE,mensaje_para_el_kernel,logs);
			log_info(logs,"CPU se desconecto por que la memoria es insuficiente");

		}break;
		case MSP_TO_CPU_TAMANIO_NEGATIVO : {

			flagError = true;
			enviarMensaje(socketKernel,MSP_TO_CPU_TAMANIO_NEGATIVO,mensaje_para_el_kernel,logs);
			log_info(logs,"CPU se desconecto por que el tamanio es negativo");

		}break;
		case MSP_TO_CPU_SEGMENTO_EXCEDE_TAMANIO_MAXIMO : {

			flagError = true;
			enviarMensaje(socketKernel,MSP_TO_CPU_SEGMENTO_EXCEDE_TAMANIO_MAXIMO,mensaje_para_el_kernel,logs);
			log_info(logs,"CPU se desconecto por que el segmento excede el tamanio maximo");

		}break;
		case MSP_TO_CPU_PID_EXCEDE_CANT_MAXIMA_DE_SEGMENTOS : {

			flagError = true;
			enviarMensaje(socketKernel,MSP_TO_CPU_PID_EXCEDE_CANT_MAXIMA_DE_SEGMENTOS,mensaje_para_el_kernel,logs);
			log_info(logs,"CPU se desconecto por que el PID excede la cantidad maxima de segmentos");

		}break;

		}



}







