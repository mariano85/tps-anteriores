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

		int32_t mi_pid = 32;

		/*	int32_t mi_pid;

		mi_pid = process_getpid ();*/


		logs = log_create("log", "CPU.c", 1, LOG_LEVEL_TRACE);

		if (argc < 2){
			log_error(logs, "No se pasaron parametros.");
			log_destroy(logs);
			return 0;
		}


		iniciarPrograma(); // Levanto el archivo de configuracion, en caso de que nose pueda tiro error
		char* ipKernel        = config_get_string_value(config, IP_KERNEL);
		char* ipMSP           = config_get_string_value(config, IP_MSP);
		int32_t puertoKernel  = config_get_int_value(config,PUERTO_KERNEL);
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

		//conexion a KERNEL
		socketKernel = conectarAServidor(ipKernel, puertoKernel);
		while (socketKernel == EXIT_FAILURE) {
			log_info(logs,
					"Despierten al Kernel! Se reintenta conexion en unos segundos\n");
			sleep(5);
			socketKernel = conectarAServidor(ipKernel, puertoKernel);
		}

		// Hago el handshake con el kernel

		t_contenido mensaje_para_enviar_al_kernel_con_el_pid;
		memset(mensaje_para_enviar_al_kernel_con_el_pid,0,sizeof(t_contenido));
		strcpy(mensaje_para_enviar_al_kernel_con_el_pid, string_from_format("%d", mi_pid));
		enviarMensaje(socketKernel,CPU_TO_KERNEL_HANDSHAKE,mensaje_para_enviar_al_kernel_con_el_pid,logs);

		//conexion a MSP
			socketMSP = conectarAServidor(ipMSP, puertoMSP);
			while (socketMSP == EXIT_FAILURE) {
				log_info(logs,
						"Despierten a la MSP! Se reintenta conexion en unos segundos \n");
				sleep(5);
				socketMSP = conectarAServidor(ipMSP, puertoMSP);
			}


		//Hago el handshake con la MSP
		enviarMensaje(socketMSP,CPU_TO_MSP_HANDSHAKE,"",logs);

		 seguir = 1;

		while(seguir){ //Este while sirve para que quede en la espera de nuevos cpu's sino haria el ciclo de ejecucion de uno solo y se me cerraria

			int contador = 0; //Verifica que el quantum no llegue a 9 (puede ejecutar 9 instrucciones)
			ejecutando = 0 ;	//No esta ejecutando aun
			log_debug(logs, "solicitando un TCB...");

			t_contenido mensaje_para_recibir_el_TCB;
			memset(mensaje_para_recibir_el_TCB,0,sizeof(t_contenido));

			t_header encabezado_recibido_por_el_kernel = recibirMensaje(socketKernel,mensaje_para_recibir_el_TCB,logs);

			if(encabezado_recibido_por_el_kernel == ERR_CONEXION_CERRADA){
				log_info(logs, "Se cerró la conexion con kernel");
				exit(EXIT_FAILURE);
			}


			if(encabezado_recibido_por_el_kernel == KERNEL_TO_CPU_TCB){

				ejecutando = 1;

				//Recibo TCB aca porque es en caso de que el kernel me mando el encabezado diciendome que me lo va a mandar
				log_info(logs, "recibir el PCB");

				char** array_para_recibir_el_TCB = string_get_string_as_array(mensaje_para_recibir_el_TCB);

				TCB = malloc(sizeof(registro_TCB));

				//Utilizo atoi() para transformar el string en numero,en teoria el kernel me lo manda ordenado para que coincida en el vector
				// Ponerse de acuerdo en el orden para las variables,sugiero que sea el orden del TP

				TCB->pid = 	atoi(array_para_recibir_el_TCB[0]);
				TCB->tid = 	atoi(array_para_recibir_el_TCB[1]);
				TCB->indicador_modo_kernel = atoi(array_para_recibir_el_TCB[2]);
				TCB->base_segmento_codigo = atoi(array_para_recibir_el_TCB[3]);
				TCB->tamanio_indice_codigo = atoi(array_para_recibir_el_TCB[4]);
				TCB->puntero_instruccion = atoi(array_para_recibir_el_TCB[5]);
				TCB->base_stack = atoi(array_para_recibir_el_TCB[6]);
				TCB->cursor_stack = atoi(array_para_recibir_el_TCB[7]);

				// Cargo los registros del TCB en los registros de la CPU, no estoy seguso si es asi

				A = TCB->registros_de_programacion.A;
				B = TCB->registros_de_programacion.B;
				C = TCB->registros_de_programacion.C;
				D = TCB->registros_de_programacion.D;
				E = TCB->registros_de_programacion.E;

				int32_t quantum = atoi(array_para_recibir_el_TCB[8]);
				int32_t offset = TCB->cursor_stack - TCB->base_stack;
				int32_t Modo = TCB->indicador_modo_kernel;

				t_contenido mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario;

				//Solicito a la MSP que me mande el stack

				memset(mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario,0,sizeof(t_contenido));
				strcpy(mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario, string_from_format("[%d,%d,%d]",TCB->pid,TCB->base_stack,offset));


				enviarMensaje(socketMSP,CPU_TO_MSP_SOLICITAR_BYTES,mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario,logs);
				log_info(logs, "Se envía mensaje a MSP para recuperar el diccionario del proceso actual");

				memset(mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario,0,sizeof(t_contenido));
				t_header mensaje_de_respuesta_a_la_solicitud_del_stack = recibirMensaje(socketMSP,mensaje_para_solicitar_stack_a_la_MSP_y_poder_cargar_el_diccionario,logs);

				if(mensaje_de_respuesta_a_la_solicitud_del_stack == ERR_CONEXION_CERRADA){
					log_info(logs, "Se cerró la conexion con la MSP");
					exit(EXIT_FAILURE);

				}

				log_info(logs, "Limpio Diccionario");
				dictionary_clean(diccionarioDeVariables);

				cargar_diccionario();

				systemCall = 0;


				while(contador < quantum && !systemCall && Modo == MODO_USUARIO){

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

				}//Fin del while(contador < quantum && !systemCall)

				while(systemCall && TCB->indicador_modo_kernel == MODO_KERNEL ){ //Para salir modifico en la instruccion XXXX el systemCall = 0

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
			liberar_estructuras_CPU();


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






