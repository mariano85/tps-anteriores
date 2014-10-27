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

int main(int argc, char *argv[]) {

	//	int32_t mi_pid = 32;

		/*	int32_t mi_pid;

		mi_pid = process_getpid ();*/

		B = 97;
		D = 3;
		C = 1;

		logs = log_create("log", "CPU.c", 1, LOG_LEVEL_TRACE);

		if (argc < 2){
			log_error(logs, "No se pasaron parametros.");
			log_destroy(logs);
			return 0;
		}


		iniciarPrograma(); // Levanto el archivo de configuracion, en caso de que nose pueda tiro error
		ipKernel   = config_get_string_value(config, IP_KERNEL);
		char* ipMSP      = config_get_string_value(config, IP_MSP);
		puertoKernel  = config_get_int_value(config,PUERTO_KERNEL);
		int32_t puertoMSP     = config_get_int_value(config, PUERTO_MSP);


		//creo diccionario de variables
		diccionarioDeVariables = dictionary_create();

			// signals
				signal(SIGINT,manejar_senial);
				signal(SIGTERM,manejar_senial);
				signal(SIGUSR1,manejar_senial);


		// me conecto al kernel y MSP
		log_info(logs, "Conexion a kernel ip:%s y puerto:%d", ipKernel, puertoKernel);
		log_info(logs, "Conexion a MSP ip:%s y puerto:%d", ipMSP, puertoMSP);





		conexion_Kernel();



		//conexion a MSP
			socketMSP = conectarAServidor(ipMSP, puertoMSP);
				while (socketMSP == EXIT_FAILURE) {
						log_info(logs,
						"Despierten a la MSP! Se reintenta conexion en unos segundos \n");
						sleep(5);
						socketMSP = conectarAServidor(ipMSP, puertoMSP);
				}


		//Hago el handshake con la MSP

		//1) Fase uno es el handshake

		t_contenido mensaje;
		memset(mensaje,0,sizeof(t_contenido));
		strcpy(mensaje,"hola");

		enviarMensaje(socketMSP,CPU_TO_MSP_HANDSHAKE,mensaje,logs);




		log_info(logs,"el newfd vale %d",newFD);

		int seguir = 1;

		while(seguir == 1){

		// Aca deberia quedarme a la escucha de que el Kernel me me mande el TCB

		t_contenido mensaje_para_recibir_TCB;
		memset(mensaje_para_recibir_TCB,0,sizeof(t_contenido));
		 recibirMensaje(newFD,mensaje_para_recibir_TCB,logs);

		 char** array = string_get_string_as_array(mensaje_para_recibir_TCB);

		 log_info(logs,"el valor del pid es %d y del program counter es %d",atoi(array[0]),atoi(array[1]));

		TCB = malloc(sizeof(registro_TCB));

		TCB->pid = atoi(array[0]); //1234;
		TCB->base_segmento_codigo = atoi(array[1]);//1048576;
		TCB->tamanio_segmento_codigo = 9;

		log_info(logs,"el valor del pid es %d",TCB->pid);
		log_info(logs,"el valor de la base es %d",TCB->base_segmento_codigo);

		program_counter = TCB->base_segmento_codigo;

		int cont = 0;


		while(cont < 5){

			t_contenido mensaje_para_solicitar_bytes_MSP;
			memset(mensaje_para_solicitar_bytes_MSP,0,sizeof(t_contenido));
			strcpy(mensaje_para_solicitar_bytes_MSP,string_from_format("[%d,%d,%d]",TCB->pid,program_counter,4));
			enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_solicitar_bytes_MSP,logs);

			usleep(60000);

			//Recibo Instruccion
			t_contenido mensaje_para_recibir_direccion;
			t_header header_verificar_mensaje = recibirMensaje(socketMSP,mensaje_para_recibir_direccion,logs);

			//Verifico que sea una instruccion

				if(header_verificar_mensaje == MSP_TO_CPU_BYTES_ENVIADOS){

					log_info(logs,"La instruccion es %s",mensaje_para_recibir_direccion);

					buscador_de_instruccion(mensaje_para_recibir_direccion);

					seguir = 0;

					//	char** array = string_get_string_as_array(mensaje_para_recibir_direccion);



					log_info(logs,"Aca salgo con el contador en %d",cont);

				}

				cont ++;

		}// Fin while cont

					log_info(logs,"ejecute dos instrucciones");

	}//Fin while seguir

	/*	t_header mensaje_MSP = recibirMensaje(socketMSP,msj,logs);

		char** vector = string_get_string_as_array(msj);

		int pid;
		int direccion;

		pid = atoi(vector[0]);
		direccion = atoi(vector[1]);

		if(mensaje_MSP == MSP_TO_CPU_BYTES_ENVIADOS)

			log_info(logs,"sali aca y los valores son : pid es %d y direccion es %d",pid,direccion);
			seguir = 0;

		}*/

	/*	t_contenido mensaje_para_recibir;
		memset(mensaje_para_recibir,0,sizeof(t_contenido));

		t_header mensaje_para_recibir_de_la_MSP = recibirMensaje(socketMSP,mensaje_para_recibir,logs);*/

	/*	 seguir = 1;

		while(seguir){ //Este while sirve para que quede en la espera de nuevos proceso sino haria el ciclo de ejecucion de uno solo y se me cerraria

			int contador = 0; //Verifica que el quantum no llegue a 9 (puede ejecutar 9 instrucciones)
			ejecutando = 0 ;	//No esta ejecutando aun
			log_debug(logs, "solicitando un TCB...");

			t_contenido mensaje_para_recibir_el_TCB;
			memset(mensaje_para_recibir_el_TCB,0,sizeof(t_contenido));

			t_header encabezado_recibido_por_el_kernel = recibirMensaje(socketKernel,mensaje_para_recibir_el_TCB,logs);

			if(encabezado_recibido_por_el_kernel == KERNEL_TO_CPU_TCB){

				ejecutando = 1;

				//Recibo TCB aca porque es en caso de que el kernel me mando el encabezado diciendome que me lo va a mandar
				log_info(logs, "recibir el TCB");

				char** array_para_recibir_el_TCB = string_get_string_as_array(mensaje_para_recibir_el_TCB);

				TCB = malloc(sizeof(registro_TCB));

				//Utilizo atoi() para transformar el string en numero,en teoria el kernel me lo manda ordenado para que coincida en el vector
				// Ponerse de acuerdo en el orden para las variables,sugiero que sea el orden del TP


				//ACLARACION : Por lo que vi en el panel debo pasar esto a una estructura como dice aca, veo despues si coinciden los nombres

				// Agregar tamaño de stack (Fe de erratas)

				TCB->pid = 	atoi(array_para_recibir_el_TCB[0]);
				TCB->tid = 	atoi(array_para_recibir_el_TCB[1]);
				TCB->indicador_modo_kernel = atoi(array_para_recibir_el_TCB[2]);
				TCB->base_segmento_codigo = atoi(array_para_recibir_el_TCB[3]);
				TCB->tamanio_segmento_codigo = atoi(array_para_recibir_el_TCB[4]);
				TCB->puntero_instruccion = atoi(array_para_recibir_el_TCB[5]);
				TCB->base_stack = atoi(array_para_recibir_el_TCB[6]);
				TCB->cursor_stack = atoi(array_para_recibir_el_TCB[7]);

				// Cargo los registros del TCB en los registros de la CPU, no estoy seguro si es asi

				A = TCB->registros_de_programacion.A;
				B = TCB->registros_de_programacion.B;
				C = TCB->registros_de_programacion.C;
				D = TCB->registros_de_programacion.D;
				E = TCB->registros_de_programacion.E;

				int32_t quantum = atoi(array_para_recibir_el_TCB[8]);
		//		int32_t offset = TCB->cursor_stack - TCB->base_stack;
				int32_t Modo = TCB->indicador_modo_kernel;

				t_contenido mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario;

				//Solicito a la MSP que me mande el stack



				memset(mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario,0,sizeof(t_contenido));
		// Mandar lo que corresponde a la interfaz de la MSP --> Dir logica		strcpy(mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario, string_from_format("[%d,%d,%d]",TCB->pid,TCB->base_stack,offset));


				enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario,logs);
				log_info(logs, "Se envía mensaje a MSP para recuperar el diccionario del proceso actual");

				memset(mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario,0,sizeof(t_contenido));
				t_header mensaje_de_respuesta_a_la_solicitud_del_stack = recibirMensaje(socketMSP,mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario,logs);

				if(mensaje_de_respuesta_a_la_solicitud_del_stack == ERR_ERROR_AL_RECIBIR_MSG){
					log_info(logs, "Se cerró la conexion con la MSP");
					exit(EXIT_FAILURE);

				}

				log_info(logs, "Limpio Diccionario");
				dictionary_clean(diccionarioDeVariables);

				cargar_diccionario();

				systemCall = 0;


				while(contador < quantum &&  Modo == MODO_USUARIO){

					//Pido la instruccion a la MSP --> Me debera pasar solo los primero 4 bytes EJ : "LOAD"

					t_contenido mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP;
					memset(mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,0,sizeof(t_contenido));
					strcpy(mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP, string_from_format("[%d,%d,%d]",TCB->pid,TCB->indice_codigo,TCB->puntero_instruccion));
					enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,logs);

					t_header mensaje_recibido_de_la_MSP_a_la_solicitud_de_la_instruccion = recibirMensaje(socketMSP,mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,logs);

					if(mensaje_recibido_de_la_MSP_a_la_solicitud_de_la_instruccion == ERR_CONEXION_CERRADA){
						log_info(logs, "Se cerró la conexion con la MSP");
						exit(EXIT_FAILURE);
					}

					program_counter = TCB->puntero_instruccion;

					char* instruccion;

					instruccion = "LOAD";

					buscador_de_instruccion(instruccion);

						if (program_counter == TCB->puntero_instruccion){ //Por si hay un llamado al sistema no aumentar el program_counter para no pisar la primer intruccion
							TCB->puntero_instruccion++;
							contador++;
							log_debug(logs, "Concluyo el quantum %d\n",contador);
							sleep((int)RETARDO/1000);

						}

				}//Fin del while(contador < quantum )

				while(TCB->indicador_modo_kernel == MODO_KERNEL && systemCall){ //Para salir modifico en la instruccion XXXX el systemCall = 0

					//Pido la instruccion a la MSP
									t_contenido mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP;
									memset(mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,0,sizeof(t_contenido));
									strcpy(mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP, string_from_format("[%d,%d,%d]",TCB->pid,TCB->indice_codigo,TCB->puntero_instruccion));
									enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,logs);

									t_header mensaje_recibido_de_la_MSP_a_la_solicitud_de_la_instruccion = recibirMensaje(socketMSP,mensaje_para_pedirle_la_proxima_instruccion_a_la_MSP,logs);

									if(mensaje_recibido_de_la_MSP_a_la_solicitud_de_la_instruccion == ERR_CONEXION_CERRADA){
										log_info(logs, "Se cerró la conexion con la MSP");
										exit(EXIT_FAILURE);
									}

									program_counter = TCB->puntero_instruccion;

									char* instruccion;

									instruccion = "LOAD";

									buscador_de_instruccion(instruccion);

									if (program_counter == TCB->puntero_instruccion) //Por si hay un llamado al sistema no aumentar el program_counter para no pisar la primer intruccion
										TCB->puntero_instruccion++;


				}//Fin while(systemCall)

				ejecutando = 0;

			}//Fin del if(encabezado_recibido_por_el_kernel == KRN_TO_CPU_TCB)

				//////////////// Finalizo el Quantum//////////////////////////////
// 	Lo que tengo que hacer es verificar si no hubo llamado al sistema en el caso de que no le devuelvo el TCB al kernel

			if(!systemCall){

				t_contenido mensaje_para_avisarle_al_kernel_que_finalizo_QUANTUM_le_envio_TCB;
				memset(mensaje_para_avisarle_al_kernel_que_finalizo_QUANTUM_le_envio_TCB,0,sizeof(t_contenido));
				strcpy(mensaje_para_avisarle_al_kernel_que_finalizo_QUANTUM_le_envio_TCB, string_from_format("[%d,%d,%d,%d,%d,%d,%d,%d]",TCB->pid,TCB->tid,TCB->indicador_modo_kernel,TCB->base_segmento_codigo,TCB->tamanio_indice_codigo,TCB->puntero_instruccion,TCB->base_stack,TCB->cursor_stack));
				enviarMensaje(socketKernel,CPU_TO_KERNEL_FINALIZO_QUANTUM_NORMALMENTE,mensaje_para_avisarle_al_kernel_que_finalizo_QUANTUM_le_envio_TCB,logs);

				t_header mensaje_respuesta_al_envio_del_TCB = recibirMensaje(socketKernel,"",logs);

				if(mensaje_respuesta_al_envio_del_TCB == ERR_ERROR_AL_ENVIAR_MSG){
					log_info(logs,"El kernel no recibio el TCB");
					exit(EXIT_FAILURE);

				}

				free(TCB);

			}

		} //Fin del while seguir

			close(socketKernel);
			close(socketMSP);
			log_info(logs, "Cerrando la CPU...");
			liberar_estructuras_CPU();*/


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

	if (offset + bytesASumar > TAMANIO_PAGINA)
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
					//continue;
				}

				log_trace(logs, "Recibí conexion de %s", inet_ntoa(their_addr.sin_addr));

				t_contenido mensajeParaRecibirConexionCpu;
				memset(mensajeParaRecibirConexionCpu, 0, sizeof(t_contenido));

				t_header header_conexion_kernel = recibirMensaje(newFD, mensajeParaRecibirConexionCpu, logs);

	//			pthread_t hilo;



				if(header_conexion_kernel == KERNEL_TO_CPU_HANDSHAKE){

			//		pthread_create(&hilo, NULL, atenderAKernel, (void*)newFD);

					log_info(logs,"HOla esasdasdasdasdsadsasdasdas");


				}

			}

		//		pthread_exit(0);




