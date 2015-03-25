/*
 * funciones_CPU.c
 *
 *  Created on: 01/10/2014
 *      Author: utnso
 */


#include "funciones_CPU.h"



/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

	void iniciarPrograma(){
		config = config_create(PROGRAMA_CONF_PATH);

		if(!config_has_property(config,IP_KERNEL)){
			puts("ERROR! FALTA EL PUERTO DEL KERNEL!");
			config_destroy(config);
			exit(-1);
		}

		if(!config_has_property(config,IP_MSP)){
			puts("ERROR! FALTA EL PUERTO DE LA MSP!");
			config_destroy(config);
			exit(-1);
		}

		if(!config_has_property(config,PUERTO_KERNEL)){
			puts("ERROR! FALTA EL PUERTO DEL KERNEL!");
			config_destroy(config);
			exit(-1);
		}

		if(!config_has_property(config,PUERTO_MSP)){

			puts("ERROR! FALTA EL PUERTO DE LA MSP");
			config_destroy(config);
			exit(-1);
		}

		if(!config_has_property(config,RETARDO)){

			puts("ERROR! FALTA EL RETARDO");
			config_destroy(config);
			exit(-1);

		}

		log_info(logs, "Archivo de configuracion levantado correctamente");

	}

//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

	void liberar_estructuras_CPU(){
			config_destroy(config);
			log_destroy(logs);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////

void buscador_de_instruccion(char* instruccion){

	P = aumentarProgramCounter(P,4);

	log_info(logs,"el program_counter es %d",P);

	//#1 LOAD : carga en el registro el numero dado

	if(string_equals_ignore_case(LOAD,instruccion)){

		int32_t auxiliar = 5;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			char* registro = malloc(2);

			int32_t numero;

			memset(registro,0,2);
			memcpy(registro,mensaje_para_recibir_bytes,1);

			memcpy(&numero,mensaje_para_recibir_bytes + 1,4);

			log_info(logs,"el registro vale %s y el numero %d",registro,numero);

			int32_t aux = verificador_de_registro(registro);

			switch(aux){

			case 0:instruccion_LOAD(&A,numero);
							break;

			case 1 : instruccion_LOAD(&B,numero);
							break;

			case 2 : instruccion_LOAD(&C,numero);
							break;

			case 3 : instruccion_LOAD(&D,numero);
							break;

			case 4 : instruccion_LOAD(&E,numero);
							break;
			case 5 : instruccion_LOAD(&M,numero);
							break;
			case 6 : instruccion_LOAD(&P,numero);
							break;
			case 7 : instruccion_LOAD(&X,numero);
							break;
			case 8 : instruccion_LOAD(&S,numero);
							break;
			}



			free(registro);

		}else{


			verificar_error_de_la_MSP(header_recibir_bytes);

		}

		estado_registros();
		P = aumentarProgramCounter(P,5);
	}

//#2 GETM : Obtiene el valor de memoria apuntado por el segundo registro.El valor obtenido lo asigna en el primer registro

	if(string_equals_ignore_case(GETM,instruccion)){

		int32_t auxiliar = 2;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);


		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

				char* registro1 = malloc(2);
				char* registro4 = malloc(2);

				memset(registro1,0,2);
				memset(registro4,0,2);

				memcpy(registro1,mensaje_para_recibir_bytes,1);
				memcpy(registro4,mensaje_para_recibir_bytes + 1,1);



			int32_t	aux1 = verificador_de_registro(registro1);
			int32_t aux2 = verificador_de_registro(registro4);

				switch(aux1){

						case 0: funcion_verificador_segundo_registro_GETM(&A,aux2);
												break;
						case 1: funcion_verificador_segundo_registro_GETM(&B,aux2);
												break;
						case 2: funcion_verificador_segundo_registro_GETM(&C,aux2);
												break;
						case 3: funcion_verificador_segundo_registro_GETM(&D,aux2);
												break;
						case 4: funcion_verificador_segundo_registro_GETM(&E,aux2);
												break;
						case 5: funcion_verificador_segundo_registro_GETM(&M,aux2);
												break;
						case 6: funcion_verificador_segundo_registro_GETM(&P,aux2);
												break;
						case 7: funcion_verificador_segundo_registro_GETM(&X,aux2);
												break;
						case 8: funcion_verificador_segundo_registro_GETM(&S,aux2);
												break;

				} //Fin del switch

				estado_registros();
				free(registro1);
				free(registro4);

		}else{


			verificar_error_de_la_MSP(header_recibir_bytes);

		}

		P = aumentarProgramCounter(P,2);

	}
	//#3 SETM : Pone tantos bytes desde el segundo registro, hacia la memoria apuntada por el primer registro

	if(string_equals_ignore_case(SETM,instruccion)){


		int32_t auxiliar_numero = 4;
		enviar_parametros(P,auxiliar_numero);

		t_contenido mensaje_para_recibir_numero;
		memset(mensaje_para_recibir_numero,0,sizeof(t_contenido));
		t_header header_recibir_numero = recibirMensaje(socketMSP,mensaje_para_recibir_numero,logs);

			if(header_recibir_numero == MSP_TO_CPU_BYTES_ENVIADOS){

				P = aumentarProgramCounter(P,4);

			}else{

				verificar_error_de_la_MSP(header_recibir_numero);


			}

		int32_t auxiliar_registros = 2;
		enviar_parametros(P,auxiliar_registros);

		t_contenido mensaje_para_recibir_registros;
		memset(mensaje_para_recibir_registros,0,sizeof(t_contenido));
		t_header header_recibir_registro = recibirMensaje(socketMSP,mensaje_para_recibir_registros,logs);

			if(header_recibir_registro == MSP_TO_CPU_BYTES_ENVIADOS){

				int32_t numero;
				memcpy(&numero,mensaje_para_recibir_numero,4);

				char* registro1 = malloc(2);
				char* registro2 = malloc(2);

				memset(registro1,0,2);
				memset(registro2,0,2);

				memcpy(registro1,mensaje_para_recibir_registros,1);
				memcpy(registro2,mensaje_para_recibir_registros + 1,1);

				int32_t aux1 = verificador_de_registro(registro1);
				int32_t aux2 = verificador_de_registro(registro2);


				void _verificador_segunda_instruccion_SETM(int32_t *var,int32_t aux2){

							switch(aux2){
								case 0 : instruccion_SETM(numero,var,&A);
											break;
								case 1 : instruccion_SETM(numero,var,&B);
											break;
								case 2 :  instruccion_SETM(numero,var,&C);
											break;
								case 3 :  instruccion_SETM(numero,var,&D);
											break;
								case 4 : instruccion_SETM(numero,var,&E);
											break;
								case 5 : instruccion_SETM(numero,var,&M);
											break;
								case 6 : instruccion_SETM(numero,var,&P);
											break;
								case 7 : instruccion_SETM(numero,var,&X);
											break;
								case 8 : instruccion_SETM(numero,var,&S);
											break;
									}
						}

				switch(aux1){

						case 0 :_verificador_segunda_instruccion_SETM(&A,aux2);
											break;
						case 1 :_verificador_segunda_instruccion_SETM(&B,aux2);
											break;
						case 2 :_verificador_segunda_instruccion_SETM(&C,aux2);
											break;
						case 3 :_verificador_segunda_instruccion_SETM(&D,aux2);
											break;
						case 4 :_verificador_segunda_instruccion_SETM(&E,aux2);
											break;
						case 5 :_verificador_segunda_instruccion_SETM(&M,aux2);
											break;
						case 6 :_verificador_segunda_instruccion_SETM(&P,aux2);
											break;
						case 7 :_verificador_segunda_instruccion_SETM(&X,aux2);
											break;
						case 8 :_verificador_segunda_instruccion_SETM(&S,aux2);
											break;

										}

					free(registro1);
					free(registro2);
					estado_registros();


			}else{

				verificar_error_de_la_MSP( header_recibir_registro);


			}

			P = aumentarProgramCounter(P,2);
		}

	//#4 MOVR : copia el valor del segundo registro al primer registro

	if(string_equals_ignore_case(MOVR,instruccion)){

		int32_t auxiliar = 2;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			char* registro1 = malloc(2);
			char* registro2 = malloc(2);

			memset(registro1,0,2);
			memset(registro2,0,2);

			memcpy(registro1,mensaje_para_recibir_bytes,1);
			memcpy(registro2,mensaje_para_recibir_bytes + 1,1);

			int32_t aux1 = verificador_de_registro(registro1);
			int32_t aux2 = verificador_de_registro(registro2);

			void _verificador_segunda_instruccion_movr(int32_t *var,int32_t aux2){

				switch(aux2){
						case 0 : instruccion_MOVR(var,&A);
										break;
						case 1 :instruccion_MOVR(var,&B);
										break;
						case 2 : instruccion_MOVR(var,&C);
										break;
						case 3 : instruccion_MOVR(var,&D);
										break;
						case 4 :instruccion_MOVR(var,&E);
										break;
						case 5 :instruccion_MOVR(var,&M);
										break;
						case 6 :instruccion_MOVR(var,&P);
										break;
						case 7 :instruccion_MOVR(var,&X);
										break;
						case 8 :instruccion_MOVR(var,&S);
										break;

							}
						}

						switch(aux1){

							case 0 :_verificador_segunda_instruccion_movr(&A,aux2);
										break;
							case 1 :_verificador_segunda_instruccion_movr(&B,aux2);
										break;
							case 2 :_verificador_segunda_instruccion_movr(&C,aux2);
										break;
							case 3 :_verificador_segunda_instruccion_movr(&D,aux2);
										break;
							case 4 :_verificador_segunda_instruccion_movr(&E,aux2);
										break;
							case 5 :_verificador_segunda_instruccion_movr(&M,aux2);
										break;
							case 6 :_verificador_segunda_instruccion_movr(&P,aux2);
																	break;
							case 7 :_verificador_segunda_instruccion_movr(&X,aux2);
																	break;
							case 8 :_verificador_segunda_instruccion_movr(&S,aux2);
																	break;
						}

					free(registro1);
					free(registro2);
					estado_registros();

		}else{

			verificar_error_de_la_MSP(header_recibir_bytes);

		}

		P = aumentarProgramCounter(P,2);

	}

	//#5 ADDR : Sume el primer registro con el segundo registro. El resultado de la operacion se almacena en el registro A

	if(string_equals_ignore_case(ADDR,instruccion)){

		int32_t auxiliar = 2;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			char* registro1 = malloc(2);
			char* registro2 = malloc(2);

			memset(registro1,0,2);
			memset(registro2,0,2);

			memcpy(registro1,mensaje_para_recibir_bytes,1);
			memcpy(registro2,mensaje_para_recibir_bytes + 1,1);

			int32_t aux1 = verificador_de_registro(registro1);
			int32_t aux2 = verificador_de_registro(registro2);

			void _verificador_segunda_instruccion_addr(int32_t *var,int32_t aux2){

				switch(aux2){
							case 0 : instruccion_ADDR(var,&A);
										break;
							case 1 :instruccion_ADDR(var,&B);
										break;
							case 2 : instruccion_ADDR(var,&C);
										break;
							case 3 : instruccion_ADDR(var,&D);
										break;
							case 4 :instruccion_ADDR(var,&E);
										break;
							case 5 :instruccion_ADDR(var,&M);
										break;
							case 6 :instruccion_ADDR(var,&P);
										break;
							case 7 :instruccion_ADDR(var,&X);
										break;
							case 8 :instruccion_ADDR(var,&S);
										break;
				}
			}

			switch(aux1){

					case 0 :_verificador_segunda_instruccion_addr(&A,aux2);
									break;
					case 1 :_verificador_segunda_instruccion_addr(&B,aux2);
									break;
					case 2 :_verificador_segunda_instruccion_addr(&C,aux2);
									break;
					case 3 :_verificador_segunda_instruccion_addr(&D,aux2);
									break;
					case 4 :_verificador_segunda_instruccion_addr(&E,aux2);
									break;
					case 5 :_verificador_segunda_instruccion_addr(&M,aux2);
														break;
					case 6 :_verificador_segunda_instruccion_addr(&P,aux2);
														break;
					case 7 :_verificador_segunda_instruccion_addr(&X,aux2);
														break;
					case 8 :_verificador_segunda_instruccion_addr(&S,aux2);
														break;
			}

			free(registro1);
			free(registro2);
			estado_registros();

		}else{

			verificar_error_de_la_MSP( header_recibir_bytes);

		}

		P = aumentarProgramCounter(P,2);

	}

	//#6 SUBR : Resta el primer registro con el segundo registro. El resultado lo almacena en el primer registro A

	if(string_equals_ignore_case(SUBR,instruccion)){

				int32_t auxiliar = 2;

				enviar_parametros(P,auxiliar);

				t_contenido mensaje_para_recibir_bytes;
				memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
				t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

				if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){


						char* registro_1 = malloc(1);
						char* registro_2 = malloc(1);

						memset(registro_1,0,2);
						memset(registro_2,0,2);


						memcpy(registro_1,mensaje_para_recibir_bytes,1);
						memcpy(registro_2,mensaje_para_recibir_bytes + 1,1);


						log_info(logs,"el valor registro1 %s",registro_1);
						log_info(logs,"el valor registro2 %s",registro_2);

						int32_t aux1 = verificador_de_registro(registro_1);
						int32_t aux2 = verificador_de_registro(registro_2);

						log_info(logs,"el valor de C es %d",C);
						log_info(logs,"el valor de D es %d",D);

						void _verificador_segunda_instruccion_subr(int32_t *var,int32_t aux2){

						switch(aux2){
							case 0 : instruccion_SUBR(var,&A);
											break;
							case 1 :instruccion_SUBR(var,&B);
											break;
							case 2 : instruccion_SUBR(var,&C);
											break;
							case 3 : instruccion_SUBR(var,&D);
											break;
							case 4 :instruccion_SUBR(var,&E);
											break;
							case 5 :instruccion_SUBR(var,&M);
																		break;
							case 6 :instruccion_SUBR(var,&P);
																		break;
							case 7 :instruccion_SUBR(var,&X);
																		break;
							case 8 :instruccion_SUBR(var,&S);
																		break;
									}
						}

						switch(aux1){
							case 0 : _verificador_segunda_instruccion_subr(&A,aux2);
											break;
							case 1 : _verificador_segunda_instruccion_subr(&B,aux2);
											break;
							case 2 : _verificador_segunda_instruccion_subr(&C,aux2);
											break;
							case 3 : _verificador_segunda_instruccion_subr(&D,aux2);
											break;
							case 4 : _verificador_segunda_instruccion_subr(&E,aux2);
											break;
							case 5 : _verificador_segunda_instruccion_subr(&M,aux2);
																		break;
							case 6 : _verificador_segunda_instruccion_subr(&P,aux2);
																		break;
							case 7 : _verificador_segunda_instruccion_subr(&X,aux2);
																		break;
							case 8 : _verificador_segunda_instruccion_subr(&S,aux2);
																		break;

						}
						free(registro_1);
						free(registro_2);
						estado_registros();
						P = aumentarProgramCounter(P,2);

				}else{

					verificar_error_de_la_MSP(header_recibir_bytes);

				}
		}

	//#7 MULR : Multiplica el primer registro con el segundo registro. El resultado de la operacion se almacena en el registro A

	if(string_equals_ignore_case(MULR,instruccion)){

			int32_t auxiliar = 2;

			enviar_parametros(P,auxiliar);

			t_contenido mensaje_para_recibir_bytes;
			memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
			t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

			if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

				char* registro_1 = malloc(1);
				char* registro_2 = malloc(1);

				memset(registro_1,0,2);
				memset(registro_2,0,2);

				memcpy(registro_1,mensaje_para_recibir_bytes,1);
				memcpy(registro_2,mensaje_para_recibir_bytes + 1,1);

				log_info(logs,"el registro1 es %s",registro_1);
				log_info(logs,"el registro2 es %s",registro_2);

				int32_t aux1 = verificador_de_registro(registro_1);
				int32_t aux2 = verificador_de_registro(registro_2);


				void _verificador_segunda_instruccion_mulr(int32_t *var,int32_t aux2){

					switch(aux2){
						case 0 : instruccion_MULR(var,&A);
									break;
						case 1 :instruccion_MULR(var,&B);
									break;
						case 2 : instruccion_MULR(var,&C);
									break;
						case 3 : instruccion_MULR(var,&D);
									break;
						case 4 :instruccion_MULR(var,&E);
									break;
						case 5 :instruccion_MULR(var,&M);
									break;
						case 6 :instruccion_MULR(var,&P);
									break;
						case 7 :instruccion_MULR(var,&X);
									break;
						case 8 :instruccion_MULR(var,&S);
									break;
								}
					}

				switch(aux1){
					case 0 : _verificador_segunda_instruccion_mulr(&A,aux2);
									break;
					case 1 : _verificador_segunda_instruccion_mulr(&B,aux2);
									break;
					case 2 : _verificador_segunda_instruccion_mulr(&C,aux2);
									break;
					case 3 : _verificador_segunda_instruccion_mulr(&D,aux2);
									break;
					case 4 : _verificador_segunda_instruccion_mulr(&E,aux2);
									break;
					case 5 : _verificador_segunda_instruccion_mulr(&M,aux2);
														break;
					case 6 : _verificador_segunda_instruccion_mulr(&P,aux2);
														break;
					case 7 : _verificador_segunda_instruccion_mulr(&X,aux2);
														break;
					case 8 : _verificador_segunda_instruccion_mulr(&S,aux2);
														break;

										}

				P = aumentarProgramCounter(P,2);
				free(registro_1);
				free(registro_2);
				estado_registros();
			}else{

				verificar_error_de_la_MSP(header_recibir_bytes);

			}


	}

	//#8 MODR : Obtiene el resto de la division del primer registro con el segundo registro. El resultado de la operacion se almacen en el registro A

	if(string_equals_ignore_case(MODR,instruccion)){


		int32_t auxiliar = 2;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){
				char* registro_1 = malloc(1);
				char* registro_2 = malloc(1);

				memset(registro_1,0,2);
				memset(registro_2,0,2);

				memcpy(registro_1,mensaje_para_recibir_bytes,1);
				memcpy(registro_2,mensaje_para_recibir_bytes + 1,1);

				log_info(logs,"el registro1 es %s",registro_1);
				log_info(logs,"el registro2 es %s",registro_2);

				int32_t aux1 = verificador_de_registro(registro_1);
				int32_t aux2 = verificador_de_registro(registro_2);


				void _verificador_segunda_instruccion_modr(int32_t *var,int32_t aux2){

					switch(aux2){
						case 0 : instruccion_MODR(var,&A);
									break;
						case 1 : instruccion_MODR(var,&B);
									break;
						case 2 :  instruccion_MODR(var,&C);
									break;
						case 3 :  instruccion_MODR(var,&D);
									break;
						case 4 : instruccion_MODR(var,&E);
									break;
						case 5 : instruccion_MODR(var,&M);
															break;
						case 6 : instruccion_MODR(var,&P);
															break;
						case 7 : instruccion_MODR(var,&X);
															break;
						case 8 : instruccion_MODR(var,&S);
															break;
								}
					}

				switch(aux1){
					case 0 : _verificador_segunda_instruccion_modr(&A,aux2);
									break;
					case 1 : _verificador_segunda_instruccion_modr(&B,aux2);
									break;
					case 2 : _verificador_segunda_instruccion_modr(&C,aux2);
									break;
					case 3 : _verificador_segunda_instruccion_modr(&D,aux2);
									break;
					case 4 : _verificador_segunda_instruccion_modr(&E,aux2);
									break;
					case 5 : _verificador_segunda_instruccion_modr(&M,aux2);
														break;
					case 6 : _verificador_segunda_instruccion_modr(&P,aux2);
														break;
					case 7 : _verificador_segunda_instruccion_modr(&X,aux2);
														break;
					case 8 : _verificador_segunda_instruccion_modr(&S,aux2);
														break;

										}

				free(registro_1);
				free(registro_2);
				P = aumentarProgramCounter(P,2);
				estado_registros();
		}else{

			verificar_error_de_la_MSP(header_recibir_bytes);

		}

	}

	//#9 DIVR : Divide el primer registro con el segundo registro. El resultado de la operacion se almacena en el registro A; a menos que el segundo operando sea 0, en cuyo caso se asigna el flag de ZERO_DIV y no se hace la operacion

	if(string_equals_ignore_case(DIVR,instruccion)){

			int32_t auxiliar = 2;

			enviar_parametros(P,auxiliar);


			t_contenido mensaje_para_recibir_bytes;
			memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
			t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

			if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){
					char* registro_1 = malloc(1);
					char* registro_2 = malloc(1);

					memset(registro_1,0,2);
					memset(registro_2,0,2);

					memcpy(registro_1,mensaje_para_recibir_bytes,1);
					memcpy(registro_2,mensaje_para_recibir_bytes + 1,1);

					log_info(logs,"el registro1 es %s",registro_1);
					log_info(logs,"el registro2 es %s",registro_2);

					int32_t aux1 = verificador_de_registro(registro_1);
					int32_t aux2 = verificador_de_registro(registro_2);


					void _verificador_segunda_instruccion_divr(int32_t *var,int32_t aux2){

						switch(aux2){
							case 0 : instruccion_DIVR(var,&A);
										break;
							case 1 : instruccion_DIVR(var,&B);
										break;
							case 2 :  instruccion_DIVR(var,&C);
										break;
							case 3 :  instruccion_DIVR(var,&D);
										break;
							case 4 : instruccion_DIVR(var,&E);
										break;
							case 5 : instruccion_DIVR(var,&M);
																	break;
							case 6 : instruccion_DIVR(var,&P);
																	break;
							case 7 : instruccion_DIVR(var,&X);
																	break;
							case 8 : instruccion_DIVR(var,&S);
																	break;
									}
						}

					switch(aux1){
						case 0 : _verificador_segunda_instruccion_divr(&A,aux2);
										break;
						case 1 : _verificador_segunda_instruccion_divr(&B,aux2);
										break;
						case 2 : _verificador_segunda_instruccion_divr(&C,aux2);
										break;
						case 3 : _verificador_segunda_instruccion_divr(&D,aux2);
										break;
						case 4 : _verificador_segunda_instruccion_divr(&E,aux2);
										break;
						case 5 : _verificador_segunda_instruccion_divr(&M,aux2);
																break;
						case 6 : _verificador_segunda_instruccion_divr(&P,aux2);
																break;
						case 7 : _verificador_segunda_instruccion_divr(&X,aux2);
																break;
						case 8 : _verificador_segunda_instruccion_divr(&S,aux2);
																break;

						}

					free(registro_1);
					free(registro_2);
					estado_registros();

				P = aumentarProgramCounter(P,2);
			}else{

				verificar_error_de_la_MSP(header_recibir_bytes);
			}
	}

	//#10 INCR : Incrementa en una unidad al registro

	if(string_equals_ignore_case(INCR,instruccion)){

		int32_t auxiliar = 1;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			char* registro_1 = malloc(2);

				memset(registro_1,0,2);
				memcpy(registro_1,mensaje_para_recibir_bytes,2);

				log_info(logs,"el registro1 es %s",registro_1);
				int32_t aux1 = verificador_de_registro(registro_1);

				switch(aux1){

				case 0:	instruccion_INCR(&A);
								break;

				case 1 :instruccion_INCR(&B);
								break;

				case 2 :instruccion_INCR(&C);
								break;

				case 3 :instruccion_INCR(&D) ;
								break;

				case 4 :instruccion_INCR(&E);
								break;
				case 5 :instruccion_INCR(&M);
												break;
				case 6 :instruccion_INCR(&P);
												break;
				case 7 :instruccion_INCR(&X);
												break;
				case 8 :instruccion_INCR(&S);
												break;

				}

		estado_registros();
		P = aumentarProgramCounter(P,1);

		}else{

			verificar_error_de_la_MSP(header_recibir_bytes);
		}

	}

	//#11 DECR : Decrementa en una unidad al registro

	if(string_equals_ignore_case(DECR,instruccion)){

		int32_t auxiliar = 1;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			char* registro_1 = malloc(1);

			memset(registro_1,0,2);
			memcpy(registro_1,mensaje_para_recibir_bytes,1);

			log_info(logs,"el registro1 es %s",registro_1);
			int32_t aux1 = verificador_de_registro(registro_1);

			switch(aux1){

				case 0:	instruccion_DECR(&A);
							break;

				case 1 :instruccion_DECR(&B);
							break;

				case 2 :instruccion_DECR(&C);
							break;

				case 3 :instruccion_DECR(&D) ;
							break;

				case 4 :instruccion_DECR(&E);
							break;
				case 5 :instruccion_DECR(&M);
											break;
				case 6 :instruccion_DECR(&P);
											break;
				case 7 :instruccion_DECR(&X);
											break;
				case 8 :instruccion_DECR(&S);
											break;

						}

				estado_registros();
				P = aumentarProgramCounter(P,1);
				free(registro_1);

			}else{

				verificar_error_de_la_MSP(header_recibir_bytes);

			}


	}

	//#12 COMP : Compara si el primer registro es igual al segundo. De ser verdadero, se almacena el valor 1.De lo contrario el valor 0.El resultado de la operacion se almacena en el registro A

	if(string_equals_ignore_case(COMP,instruccion)){

			int32_t auxiliar = 2;

			enviar_parametros(P,auxiliar);

			t_contenido mensaje_para_recibir_bytes;
			memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
			t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

			if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

				char* registro_1 = malloc(1);
				char* registro_2 = malloc(1);

				memset(registro_1,0,2);
				memset(registro_2,0,2);

				memcpy(registro_1,mensaje_para_recibir_bytes,1);
				memcpy(registro_2,mensaje_para_recibir_bytes + 1,1);

				log_info(logs,"el registro1 es %s",registro_1);
				log_info(logs,"el registro2 es %s",registro_2);

				int32_t aux1 = verificador_de_registro(registro_1);
				int32_t aux2 = verificador_de_registro(registro_2);

				void _verificador_segunda_instruccion_comp(int32_t *var,int32_t aux2){

						switch(aux2){
							case 0 : instruccion_COMP(var,&A);
											break;
							case 1 : instruccion_COMP(var,&B);
											break;
							case 2 :  instruccion_COMP(var,&C);
											break;
							case 3 :  instruccion_COMP(var,&D);
											break;
							case 4 : instruccion_COMP(var,&E);
											break;
							case 5 : instruccion_COMP(var,&M);
																		break;
							case 6 : instruccion_COMP(var,&P);
																		break;
							case 7 : instruccion_COMP(var,&X);
																		break;
							case 8 : instruccion_COMP(var,&S);
																		break;

								}
					}

				switch(aux1){
					case 0 : _verificador_segunda_instruccion_comp(&A,aux2);
										break;
					case 1 : _verificador_segunda_instruccion_comp(&B,aux2);
										break;
					case 2 : _verificador_segunda_instruccion_comp(&C,aux2);
										break;
					case 3 : _verificador_segunda_instruccion_comp(&D,aux2);
										break;
					case 4 : _verificador_segunda_instruccion_comp(&E,aux2);
										break;
					case 5 : _verificador_segunda_instruccion_comp(&M,aux2);
															break;
					case 6 : _verificador_segunda_instruccion_comp(&P,aux2);
															break;
					case 7 : _verificador_segunda_instruccion_comp(&X,aux2);
															break;
					case 8 : _verificador_segunda_instruccion_comp(&S,aux2);
															break;

								}


				estado_registros();
				P = aumentarProgramCounter(P,2);
				free(registro_1);
				free(registro_2);


			}else{


				verificar_error_de_la_MSP(header_recibir_bytes);

			}

	}

	//#13 CGEQ : Compara si el primer registro es mayor o igual al segundo. De ser verdadero, se almacena el valor 1.Delo contrario el valor 0.El resultado de la operacion se almacen en el registro A

	if(string_equals_ignore_case(CGEQ,instruccion)){

		int32_t auxiliar = 2;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			char* registro_1 = malloc(1);
			char* registro_2 = malloc(1);

			memset(registro_1,0,2);
			memset(registro_2,0,2);

			memcpy(registro_1,mensaje_para_recibir_bytes,1);
			memcpy(registro_2,mensaje_para_recibir_bytes + 1,1);

			log_info(logs,"el registro1 es %s",registro_1);
			log_info(logs,"el registro2 es %s",registro_2);

			int32_t aux1 = verificador_de_registro(registro_1);
			int32_t aux2 = verificador_de_registro(registro_2);

			log_info(logs,"el valor del registro A es %d antes de ejecutar la instruccion",A);

			void _verificador_segunda_instruccion_cgeq(int32_t *var,int32_t aux2){

					switch(aux2){
						case 0 : instruccion_CGEQ(var,&A);
										break;
						case 1 : instruccion_CGEQ(var,&B);
										break;
						case 2 :  instruccion_CGEQ(var,&C);
										break;
						case 3 :  instruccion_CGEQ(var,&D);
										break;
						case 4 : instruccion_CGEQ(var,&E);
										break;
						case 5 : instruccion_CGEQ(var,&M);
																break;
						case 6 : instruccion_CGEQ(var,&P);
																break;
						case 7 : instruccion_CGEQ(var,&X);
																break;
						case 8 : instruccion_CGEQ(var,&S);
																break;
							}

				}

					switch(aux1){
						case 0 : _verificador_segunda_instruccion_cgeq(&A,aux2);
										break;
						case 1 : _verificador_segunda_instruccion_cgeq(&B,aux2);
										break;
						case 2 : _verificador_segunda_instruccion_cgeq(&C,aux2);
										break;
						case 3 : _verificador_segunda_instruccion_cgeq(&D,aux2);
										break;
						case 4 : _verificador_segunda_instruccion_cgeq(&E,aux2);
										break;
						case 5 : _verificador_segunda_instruccion_cgeq(&M,aux2);
																break;
						case 6 : _verificador_segunda_instruccion_cgeq(&P,aux2);
																break;
						case 7 : _verificador_segunda_instruccion_cgeq(&X,aux2);
																break;
						case 8 : _verificador_segunda_instruccion_cgeq(&S,aux2);
																break;

								}


					estado_registros();
					P = aumentarProgramCounter(P,2);
					free(registro_1);
					free(registro_2);


		}else{


			verificar_error_de_la_MSP(header_recibir_bytes);


		}

	}

	//#14 CLEQ : Compara si el primer registro es menor o igual al segundo. De ser verdadero, se almacena el valor 1.Delo contrario el valor 0.El resultado de la operacion se almacen en el registro A

	if(string_equals_ignore_case(CLEQ,instruccion)){

		int32_t auxiliar = 2;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			char* registro_1 = malloc(1);
			char* registro_2 = malloc(1);

			memset(registro_1,0,2);
			memset(registro_2,0,2);

			memcpy(registro_1,mensaje_para_recibir_bytes,1);
			memcpy(registro_2,mensaje_para_recibir_bytes + 1,1);

			log_info(logs,"el registro1 es %s",registro_1);
			log_info(logs,"el registro2 es %s",registro_2);

			int32_t aux1 = verificador_de_registro(registro_1);
			int32_t aux2 = verificador_de_registro(registro_2);

			log_info(logs,"el valor del registro A es %d antes de ejecutar la instruccion",A);

			void _verificador_segunda_instruccion_cleq(int32_t *var,int32_t aux2){

					switch(aux2){
						case 0 : instruccion_CLEQ(var,&A);
										break;
						case 1 :instruccion_CLEQ(var,&B);
										break;
						case 2 : instruccion_CLEQ(var,&C);
										break;
						case 3 :  instruccion_CLEQ(var,&D);
										break;
						case 4 : instruccion_CLEQ(var,&E);
										break;
						case 5 : instruccion_CLEQ(var,&M);
															break;
						case 6 : instruccion_CLEQ(var,&P);
															break;
						case 7 : instruccion_CLEQ(var,&X);
															break;
						case 8 : instruccion_CLEQ(var,&S);
															break;
						}
				}

					switch(aux1){
						case 0 : _verificador_segunda_instruccion_cleq(&A,aux2);
										break;
						case 1 : _verificador_segunda_instruccion_cleq(&B,aux2);
										break;
						case 2 : _verificador_segunda_instruccion_cleq(&C,aux2);
										break;
						case 3 : _verificador_segunda_instruccion_cleq(&D,aux2);
										break;
						case 4 : _verificador_segunda_instruccion_cleq(&E,aux2);
										break;
						case 5 : _verificador_segunda_instruccion_cleq(&M,aux2);
																break;
						case 6 : _verificador_segunda_instruccion_cleq(&P,aux2);
																break;
						case 7 : _verificador_segunda_instruccion_cleq(&X,aux2);
																break;
						case 8 : _verificador_segunda_instruccion_cleq(&S,aux2);
																break;

						}

							P = aumentarProgramCounter(P,2);
							free(registro_1);
							free(registro_2);
							estado_registros();

				}else{


					verificar_error_de_la_MSP(header_recibir_bytes);

				}

	}

	//#15 GOTO : Altera el flujo de ejecucion para ejecutar la instruccion apuntada por el registro. El valor es el desplazamiento desde el inicio del programa

	if(string_equals_ignore_case(GOTO,instruccion)){

		int32_t auxiliar = 1;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			char* registro_1 = malloc(1);

			memset(registro_1,0,2);
			memcpy(registro_1,mensaje_para_recibir_bytes,1);

			log_info(logs,"el registro1 es %s",registro_1);
			int32_t aux1 = verificador_de_registro(registro_1);

			switch(aux1){

				case 0:	instruccion_GOTO(&A);
								break;
				case 1 :instruccion_GOTO(&B);
								break;
				case 2 :instruccion_GOTO(&C);
								break;
				case 3 :instruccion_GOTO(&D) ;
								break;
				case 4 :instruccion_GOTO(&E);
								break;
				case 5 :instruccion_GOTO(&M);
											break;
				case 6 :instruccion_GOTO(&P);
											break;
				case 7 :instruccion_GOTO(&X);
											break;
				case 8 :instruccion_GOTO(&S);
											break;

					}
				estado_registros();
				free(registro_1);
		}else{


			verificar_error_de_la_MSP(header_recibir_bytes);

		}
}

	//#16 JMPZ : Altera el flujo de ejecucion, solo si el valor del registro A es cero,para ejecutar la instruccion apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.

	if(string_equals_ignore_case(JMPZ,instruccion)){

			int32_t auxiliar = 4;

			enviar_parametros(P,auxiliar);



			t_contenido mensaje_para_recibir_bytes;
			memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
			t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

			if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

				int32_t direccion = 0;

				memcpy(&direccion,mensaje_para_recibir_bytes,4);

				log_info(logs,"el registro1 es %d",direccion);

				instruccion_JMPZ(direccion);

				estado_registros();

			}else{


				verificar_error_de_la_MSP(header_recibir_bytes);

			}
	}

	//#17 JPNZ : Altera el flujo de ejecucion, solo si el valor del registro A no es cero,para ejecutar la instruccion apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.

	if(string_equals_ignore_case(JPNZ,instruccion)){


		int32_t auxiliar = 4;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			int32_t direccion = 0;

			memcpy(&direccion,mensaje_para_recibir_bytes,4);

			log_info(logs,"el registro1 es %d",direccion);

			instruccion_JPNZ(direccion);

			estado_registros();

		}else{


			verificar_error_de_la_MSP(header_recibir_bytes);

		}


	}

	//#18 INTE[Direccion]
	//Interrumpe la ejecución del programa para ejecutar la rutina del kernel que se encuentra en la
	//posición apuntada por la direccion. El ensamblador admite ingresar una cadena indicando el
	//nombre, que luego transformará en el número correspondiente. Los posibles valores son
	//“MALC”, “FREE”, “INNN”, “INNC”, “OUTN”, “OUTC”, “BLOK”, “WAKE”, “CREA” y “JOIN”. Invoca al
	//servicio correspondiente en el proceso Kernel. Notar que el hilo en cuestión debe bloquearse
	//tras una interrupción.

	if(string_equals_ignore_case(INTE,instruccion)){

		int32_t auxiliar = 4;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			int32_t direccion_recibida;

			memcpy(&direccion_recibida,mensaje_para_recibir_bytes,4);
			instruccion_INTE(direccion_recibida);

		}else{


			verificar_error_de_la_MSP(header_recibir_bytes);

		}

		estado_registros();

	}

	//#20 SHIF [numero][registro]
	//Desplaza los bits del registro, tantas veces como se indique en el numero. De ser desplazamiento positivo, se considera hacia la derecha. De lo contrario hacia la izquierda

	if(string_equals_ignore_case(SHIF,instruccion)){

		int32_t auxiliar = 4;

		enviar_parametros(P,auxiliar);

		t_contenido mensaje_para_recibir_numero;
		memset(mensaje_para_recibir_numero,0,sizeof(t_contenido));
		t_header header_recibir_numero = recibirMensaje(socketMSP,mensaje_para_recibir_numero,logs);

			if(header_recibir_numero == MSP_TO_CPU_BYTES_ENVIADOS){

				P = aumentarProgramCounter(P,4);

			}else{


				verificar_error_de_la_MSP(header_recibir_numero);

			}

			int32_t auxiliar_registro = 1;
			enviar_parametros(P,auxiliar_registro);

			t_contenido mensaje_para_recibir_registro;
			memset(mensaje_para_recibir_registro,0,sizeof(t_contenido));
			t_header header_recibir_registro = recibirMensaje(socketMSP,mensaje_para_recibir_registro,logs);

			if( header_recibir_registro == MSP_TO_CPU_BYTES_ENVIADOS){

					char* registro = malloc(2);
					int32_t numero;

					memset(registro,0,2);
					memcpy(&numero,mensaje_para_recibir_numero,4);
					memcpy(registro,mensaje_para_recibir_registro,2);

					int32_t aux = verificador_de_registro(registro);

					switch(aux){

						case 0:instruccion_SHIF(&A,numero);
										break;
						case 1 : instruccion_SHIF(&B,numero);
										break;
						case 2 : instruccion_SHIF(&C,numero);
										break;
						case 3 : instruccion_SHIF(&D,numero);
										break;
						case 4 : instruccion_SHIF(&E,numero);
										break;
						case 5 : instruccion_SHIF(&M,numero);
																break;
						case 6 : instruccion_SHIF(&P,numero);
																break;
						case 7 : instruccion_SHIF(&X,numero);
																break;
						case 8 : instruccion_SHIF(&S,numero);
																break;
						}

					free(registro);


			}else{

				verificar_error_de_la_MSP(header_recibir_registro);

			}


			estado_registros();
			P = aumentarProgramCounter(P,1);

	}

	//#21 NOPP
	//Consume un ciclo del CPU sin hacer nada

	if(string_equals_ignore_case(NOPP,instruccion)){

		log_info(logs,"Consumo un Quantum");
		estado_registros();

	}

	//#22 PUSH
	//Apila los primeros bytes, indicando por el numero del registro hacia el stack. Modifica el valor del registro cursor de stack de forma acorde

	if(string_equals_ignore_case(PUSH,instruccion)){

		log_info(logs,"entre al push");

		int32_t auxiliar_numero = 4;
		enviar_parametros(P,auxiliar_numero);

		t_contenido mensaje_para_recibir_numero;
		memset(mensaje_para_recibir_numero,0,sizeof(t_contenido));
		t_header header_recibir_numero = recibirMensaje(socketMSP,mensaje_para_recibir_numero,logs);

		if(header_recibir_numero == MSP_TO_CPU_BYTES_ENVIADOS){

			P = aumentarProgramCounter(P,4);

		}else{

			verificar_error_de_la_MSP(header_recibir_numero);

		}

		int32_t auxiliar_registro = 1;
		enviar_parametros(P,auxiliar_registro);

		t_contenido mensaje_para_recibir_registro;
		memset(mensaje_para_recibir_registro,0,sizeof(t_contenido));
		t_header header_recibir_registro = recibirMensaje(socketMSP,mensaje_para_recibir_registro,logs);

		if(header_recibir_registro == MSP_TO_CPU_BYTES_ENVIADOS){

			log_info(logs,"voy a ejecutar push");

			char* registro = malloc(2);
			int32_t numero;

			memset(registro,0,2);
			memcpy(&numero,mensaje_para_recibir_numero,4);
			memcpy(registro,mensaje_para_recibir_registro,2);

			int32_t aux = verificador_de_registro(registro);

			log_info(logs,"el valor del numero es %d y el registro es %s",numero,registro);

				switch(aux){

				case 0:instruccion_PUSH(A,numero);
								break;
				case 1 : instruccion_PUSH(B,numero);
								break;
				case 2 : instruccion_PUSH(C,numero);
								break;
				case 3 : instruccion_PUSH(D,numero);
								break;
				case 4 : instruccion_PUSH(E,numero);
								break;
				case 5 : instruccion_PUSH(M,numero);
								break;
				case 6 : instruccion_PUSH(P,numero);
								break;
				case 7 : instruccion_PUSH(X,numero);
								break;
				case 8 : instruccion_PUSH(S,numero);
								break;

				}

				P = aumentarProgramCounter(P,1);
				free(registro);
		}else{

			verificar_error_de_la_MSP(header_recibir_registro);

		}



	}
	//#23 TAKE

	if(string_equals_ignore_case(TAKE,instruccion)){

		log_info(logs,"entre al take");

		int32_t auxiliar_numero = 4;
		enviar_parametros(P,auxiliar_numero);

		t_contenido mensaje_para_recibir_numero;
		memset(mensaje_para_recibir_numero,0,sizeof(t_contenido));
		t_header header_recibir_numero = recibirMensaje(socketMSP,mensaje_para_recibir_numero,logs);

		if(header_recibir_numero == MSP_TO_CPU_BYTES_ENVIADOS){

			P = aumentarProgramCounter(P,4);

		}else{


			verificar_error_de_la_MSP(header_recibir_numero);

		}

		int32_t auxiliar_registro = 1;
		enviar_parametros(P,auxiliar_registro);

		t_contenido mensaje_para_recibir_registro;
		memset(mensaje_para_recibir_registro,0,sizeof(t_contenido));
		t_header header_recibir_registro = recibirMensaje(socketMSP,mensaje_para_recibir_registro,logs);

			if(header_recibir_registro == MSP_TO_CPU_BYTES_ENVIADOS){

				log_info(logs,"voy a ejecutar take");

				char* registro = malloc(2);
				int32_t numero;

				memset(registro,0,2);
				memcpy(&numero,mensaje_para_recibir_numero,4);
				memcpy(registro,mensaje_para_recibir_registro,2);

				int32_t aux = verificador_de_registro(registro);

				log_info(logs,"el valor del numero es %d y el registro es %s",numero,registro);

				switch(aux){

					case 0:instruccion_TAKE(&A,numero);
									break;
					case 1 : instruccion_TAKE(&B,numero);
									break;
					case 2 : instruccion_TAKE(&C,numero);
									break;
					case 3 : instruccion_TAKE(&D,numero);
									break;
					case 4 : instruccion_TAKE(&E,numero);
									break;
					case 5 : instruccion_TAKE(&M,numero);
									break;
					case 6 : instruccion_TAKE(&P,numero);
									break;
					case 7 : instruccion_TAKE(&X,numero);
									break;
					case 8 : instruccion_TAKE(&S,numero);
									break;

							}

				P = aumentarProgramCounter(P,1);
				free(registro);

			}else{

				verificar_error_de_la_MSP(header_recibir_registro);

			}
	}

	//#24 XXXX
	if(string_equals_ignore_case(XXXX,instruccion)){

		instruccion_XXXX();

		cont = QUANTUM;
		termino_proceso_XXXX = 0;
		estado_registros();

		}

	//#25 MALC

	if(string_equals_ignore_case(MALC,instruccion)){

		instruccion_MALC();
		estado_registros();
	}

	//#26 FREE

	if(string_equals_ignore_case(FREE,instruccion)){


				instruccion_FREE();
				estado_registros();
	}


	//#27 INNN

		if(string_equals_ignore_case(INNN,instruccion)){

					instruccion_INNN();
					estado_registros();
		}

	//#28 INNC

	if(string_equals_ignore_case(INNC,instruccion)){

		instruccion_INNC();
		estado_registros();

	}



	//#29 OUTN
		if(string_equals_ignore_case(OUTN,instruccion)){

			instruccion_OUTN();
			estado_registros();
		}

	//#30 OUTC
	if(string_equals_ignore_case(OUTC,instruccion)){

			instruccion_OUTC();
			estado_registros();
	}

	//#31 CREA
	if(string_equals_ignore_case(CREA,instruccion)){

		instruccion_CREA();
		estado_registros();


	}

	//#32 JOIN
	if(string_equals_ignore_case(JOIN,instruccion)){

		instruccion_JOIN();
		estado_registros();

	}

	//#33 BLOK
	if(string_equals_ignore_case(BLOK,instruccion)){

		instruccion_BLOK();
		estado_registros();
	}

	//#32 WAKE
	if(string_equals_ignore_case(WAKE,instruccion)){

		instruccion_WAKE();
		estado_registros();

	}


}


//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
char* recibir_Instruccion(){

	char* puntero;

	puntero = malloc(sizeof(char));



	return puntero;

}

//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

int32_t verificador_de_registro(char* valor){

	if(string_equals_ignore_case(valor,registro_A)){

			return 0;
	}

	if(string_equals_ignore_case(valor,registro_B)){

				return 1;
		}

	if(string_equals_ignore_case(valor,registro_C)){

				return 2;
		}

	if(string_equals_ignore_case(valor,registro_D)){

					return 3;
			}

	if(string_equals_ignore_case(valor,registro_E)){

						return 4;
				}
	if(string_equals_ignore_case(valor,registro_M)){

						return 5;
	}

	if(string_equals_ignore_case(valor,registro_P)){
						return 6;
	}

	if(string_equals_ignore_case(valor,registro_X)){

						return 7;
	}

	if(string_equals_ignore_case(valor,registro_S)){

						return 8;
	}
	return -1;
}

void funcion_verificador_segundo_registro_GETM(int32_t *var,int32_t aux2){

					switch(aux2){
							case 0 : instruccion_GETM(var,A);
								break;
							case 1 :
								instruccion_GETM(var,B);
								break;
							case 2 : instruccion_GETM(var,C);
								break;
							case 3 : instruccion_GETM(var,D);
								break;
							case 4 : instruccion_GETM(var,E);
								break;
							case 5 : instruccion_GETM(var,M);
								break;
							case 6 : instruccion_GETM(var,P);
								break;
							case 7 : instruccion_GETM(var,X);
								break;
							case 8 : instruccion_GETM(var,S);
								break;

						}
					}



void enviar_parametros(int32_t program_counter,int32_t auxiliar){

	t_contenido mensaje_cantidad_bytes;
	memset(mensaje_cantidad_bytes,0,sizeof(t_contenido));
	strcpy(mensaje_cantidad_bytes, string_from_format("[%d,%d]",P,auxiliar));
	enviarMensaje(socketMSP,CPU_TO_MSP_PARAMETROS,mensaje_cantidad_bytes,logs);
}

void estado_registros(){


	log_info(logs,"el valor de los registros\n\tA:\t%d\n\tB:\t%d\n\tC:\t%d\n\tD:\t%d\n\tE:\t%d\n\tX:\t%d\n\tS:\t%d\n\tP:\t%d\n\tM:\t%d",A,B,C,D,E,X,S,P,M);


}

