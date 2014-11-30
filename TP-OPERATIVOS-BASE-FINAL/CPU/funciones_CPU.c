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

	program_counter = aumentarProgramCounter(program_counter,4);

	log_info(logs,"el program_counter es %d",program_counter);

	//#1 LOAD : carga en el registro el numero dado

	if(string_equals_ignore_case(LOAD,instruccion)){

		int32_t auxiliar = 5;

		enviar_parametros(program_counter,auxiliar);

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

			}

			log_info(logs,"el valor del registro %s es %d",registro,numero);

			free(registro);

		}

		program_counter = aumentarProgramCounter(program_counter,5);
	}

//#2 GETM : Obtiene el valor de memoria apuntado por el segundo registro.El valor obtenido lo asigna en el primer registro

	if(string_equals_ignore_case(GETM,instruccion)){

		int32_t auxiliar = 2;

		enviar_parametros(program_counter,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);


		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

				char* registro1 = malloc(1);
				char* registro4 = malloc(1);

				memset(registro1,0,2);
				memset(registro4,0,2);

				memcpy(registro1,mensaje_para_recibir_bytes,1);
				memcpy(registro4,mensaje_para_recibir_bytes + 1,2);



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
				} //Fin del switch

				log_info(logs,"el valor del registro A es %d",A);
				free(registro1);
				free(registro4);

		} //Fin del If

		program_counter = aumentarProgramCounter(program_counter,2);

	}
	//#3 SETM : Pone tantos bytes desde el segundo registro, hacia la memoria apuntada por el primer registro

/*	if(string_equals_ignore_case(SETM,instruccion)){

			int32_t numero = solicitar_numero();

			int32_t registro1 = solicitar_registro();

			int32_t registro2 = solicitar_registro();

			instruccion_SETM(numero,registro1,registro2);

		}*/

	//#4 MOVR : copia el valor del segundo registro al primer registro

	if(string_equals_ignore_case(MOVR,instruccion)){

		int32_t auxiliar = 2;

		enviar_parametros(program_counter,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			char* registro1 = malloc(1);
			char* registro2 = malloc(1);

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
						}

					free(registro1);
					free(registro2);

		}// Fin del if

		program_counter = aumentarProgramCounter(program_counter,2);

	}

	//#5 ADDR : Sume el primer registro con el segundo registro. El resultado de la operacion se almacena en el registro A

	if(string_equals_ignore_case(ADDR,instruccion)){

		int32_t auxiliar = 2;

		enviar_parametros(program_counter,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

			char* registro1 = malloc(1);
			char* registro2 = malloc(1);

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
									log_info(logs,"entro en el case 3");
										break;
							case 4 :instruccion_ADDR(var,&E);
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
			}

			log_info(logs,"el valor deberia dar 100 del registro B y da :%d",B);

			free(registro1);
			free(registro2);
		}

		program_counter = aumentarProgramCounter(program_counter,2);

	}

	//#6 SUBR : Resta el primer registro con el segundo registro. El resultado lo almacena en el primer registro A

	if(string_equals_ignore_case(SUBR,instruccion)){

				int32_t auxiliar = 2;

				enviar_parametros(program_counter,auxiliar);

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
							log_info(logs,"entro en el case 2");
											break;
							case 3 : instruccion_SUBR(var,&D);
											break;
							case 4 :instruccion_SUBR(var,&E);
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

						}
						free(registro_1);
						free(registro_2);
						log_info(logs,"el valor del registro d es %d",D);

						program_counter = aumentarProgramCounter(program_counter,2);

				}
		}

	//#7 MULR : Multiplica el primer registro con el segundo registro. El resultado de la operacion se almacena en el registro A

	if(string_equals_ignore_case(MULR,instruccion)){

			int32_t auxiliar = 2;

			enviar_parametros(program_counter,auxiliar);

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
						log_info(logs,"entro en el case 2");
									break;
						case 2 : instruccion_MULR(var,&C);
									break;
						case 3 : instruccion_MULR(var,&D);
									break;
						case 4 :instruccion_MULR(var,&E);
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

										}

				log_info(logs,"el valor del registro A deberia ser 9700 y es : %d",A);
				program_counter = aumentarProgramCounter(program_counter,2);
				free(registro_1);
				free(registro_2);
			}


	}

	//#8 MODR : Obtiene el resto de la division del primer registro con el segundo registro. El resultado de la operacion se almacen en el registro A

	if(string_equals_ignore_case(MODR,instruccion)){


		int32_t auxiliar = 2;

		enviar_parametros(program_counter,auxiliar);

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

										}

				log_info(logs,"el valor del registro tiene que ser 1 y es %d",D);

				free(registro_1);
				free(registro_2);
				program_counter = aumentarProgramCounter(program_counter,2);
		}

	}

	//#9 DIVR : Divide el primer registro con el segundo registro. El resultado de la operacion se almacena en el registro A; a menos que el segundo operando sea 0, en cuyo caso se asigna el flag de ZERO_DIV y no se hace la operacion

	if(string_equals_ignore_case(DIVR,instruccion)){

			int32_t auxiliar = 2;

			enviar_parametros(program_counter,auxiliar);


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

						}

					log_info(logs,"el valor de E es %d",E);

					free(registro_1);
					free(registro_2);

				program_counter = aumentarProgramCounter(program_counter,2);
			}
	}

	//#10 INCR : Incrementa en una unidad al registro

	if(string_equals_ignore_case(INCR,instruccion)){

		int32_t auxiliar = 1;

		enviar_parametros(program_counter,auxiliar);

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

				}

		log_info(logs,"el valor de A es %d",A);
		program_counter = aumentarProgramCounter(program_counter,1);

		}

	}

	//#11 DECR : Decrementa en una unidad al registro

	if(string_equals_ignore_case(DECR,instruccion)){

		int32_t auxiliar = 1;

		enviar_parametros(program_counter,auxiliar);

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

						}

				log_info(logs,"el valor de A es %d",A);
				program_counter = aumentarProgramCounter(program_counter,1);
				free(registro_1);

			}


	}

	//#12 COMP : Compara si el primer registro es igual al segundo. De ser verdadero, se almacena el valor 1.De lo contrario el valor 0.El resultado de la operacion se almacena en el registro A

	if(string_equals_ignore_case(COMP,instruccion)){

			int32_t auxiliar = 2;

			enviar_parametros(program_counter,auxiliar);

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

								}


				log_info(logs,"el valor de A es %d",A);
				program_counter = aumentarProgramCounter(program_counter,2);
				free(registro_1);
				free(registro_2);


			}

	}

	//#13 CGEQ : Compara si el primer registro es mayor o igual al segundo. De ser verdadero, se almacena el valor 1.Delo contrario el valor 0.El resultado de la operacion se almacen en el registro A

	if(string_equals_ignore_case(CGEQ,instruccion)){

		int32_t auxiliar = 2;

		enviar_parametros(program_counter,auxiliar);

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

								}


					log_info(logs,"el valor de A es %d",A);
					program_counter = aumentarProgramCounter(program_counter,2);
					free(registro_1);
					free(registro_2);


		}

	}

	//#14 CLEQ : Compara si el primer registro es menor o igual al segundo. De ser verdadero, se almacena el valor 1.Delo contrario el valor 0.El resultado de la operacion se almacen en el registro A

	if(string_equals_ignore_case(CLEQ,instruccion)){

		int32_t auxiliar = 2;

		enviar_parametros(program_counter,auxiliar);

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

						}

							log_info(logs,"el valor de A es %d",A);
							program_counter = aumentarProgramCounter(program_counter,2);
							free(registro_1);
							free(registro_2);


				}

	}

	//#15 GOTO : Altera el flujo de ejecucion para ejecutar la instruccion apuntada por el registro. El valor es el desplazamiento desde el inicio del programa

	if(string_equals_ignore_case(GOTO,instruccion)){

		int32_t auxiliar = 1;

		enviar_parametros(program_counter,auxiliar);

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

					}

				log_info(logs,"el valor de A es %d",A);
				free(registro_1);
		}
}

	//#16 JMPZ : Altera el flujo de ejecucion, solo si el valor del registro A es cero,para ejecutar la instruccion apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.

	if(string_equals_ignore_case(JMPZ,instruccion)){

			int32_t auxiliar = 1;

			enviar_parametros(program_counter,auxiliar);

			t_contenido mensaje_para_recibir_bytes;
			memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
			t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

			if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){

				char* direccion_char = malloc(4);

				memset(direccion_char,0,4);
				memcpy(direccion_char,mensaje_para_recibir_bytes,4);

				log_info(logs,"el registro1 es %s",direccion_char);
				int32_t direccion = atoi(direccion_char);

				instruccion_JMPZ(direccion);


				free(direccion_char);
			}
	}

	//#17 JPNZ : Altera el flujo de ejecucion, solo si el valor del registro A no es cero,para ejecutar la instruccion apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.

	if(string_equals_ignore_case(JPNZ,instruccion)){


		int32_t auxiliar = 4;

		enviar_parametros(program_counter,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_ENVIADOS){


			log_info(logs,"el valor de la direccion es %d",(int32_t) mensaje_para_recibir_bytes);

			instruccion_JPNZ((int32_t)mensaje_para_recibir_bytes);



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

		enviar_parametros(program_counter,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

		if(header_recibir_bytes == MSP_TO_CPU_BYTES_RECIBIDOS){

			log_info(logs,"voy a ejecutar una instruccion privilegiada");

			int32_t direccion_recibida;

			memcpy(&direccion_recibida,mensaje_para_recibir_bytes,4);

			log_info(logs,"el registro1 es %d",direccion_recibida);

			instruccion_INTE(direccion_recibida);

		}
	}

	//#20 SHIF [numero][registro]
	//Desplaza los bits del registro, tantas veces como se indique en el numero. De ser desplazamiento positivo, se considera hacia la derecha. De lo contrario hacia la izquierda

	if(string_equals_ignore_case(SHIF,instruccion)){

		int32_t auxiliar = 5;

		enviar_parametros(program_counter,auxiliar);

		t_contenido mensaje_para_recibir_bytes;
		memset(mensaje_para_recibir_bytes,0,sizeof(t_contenido));
		t_header header_recibir_bytes = recibirMensaje(socketMSP,mensaje_para_recibir_bytes,logs);

			if(header_recibir_bytes == MSP_TO_CPU_BYTES_RECIBIDOS){

					log_info(logs,"voy a ejecutar la instruccion SHIF");

					char* registro = malloc(2);
					int32_t numero;

					memset(registro,0,2);
					memcpy(&numero,mensaje_para_recibir_bytes,4);
					memcpy(registro,mensaje_para_recibir_bytes + 4,1);

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
						}





								free(registro);

			}
	}


	//#22 PUSH
	//Apila los primeros bytes, indicando por el numero del registro hacia el stack. Modifica el valor del registro cursor de stack de forma acorde

	if(string_equals_ignore_case(PUSH,instruccion)){

		log_info(logs,"entre al push");

		int32_t auxiliar_numero = 4;
		enviar_parametros(program_counter,auxiliar_numero);

		t_contenido mensaje_para_recibir_numero;
		memset(mensaje_para_recibir_numero,0,sizeof(t_contenido));
		t_header header_recibir_numero = recibirMensaje(socketMSP,mensaje_para_recibir_numero,logs);

		if(header_recibir_numero == MSP_TO_CPU_BYTES_ENVIADOS){

			program_counter = aumentarProgramCounter(program_counter,4);

		}

		int32_t auxiliar_registro = 1;
		enviar_parametros(program_counter,auxiliar_registro);

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

				case 0:instruccion_PUSH(&A,numero);
								break;
				case 1 : instruccion_PUSH(&B,numero);
								break;

				case 2 : instruccion_PUSH(&C,numero);
						log_info(logs,"entre en C");

								break;

				case 3 : instruccion_PUSH(&D,numero);
								break;

				case 4 : instruccion_PUSH(&E,numero);
								break;

				}

				program_counter = aumentarProgramCounter(program_counter,1);
			free(registro);
		}



	}
	//#TAKE

	if(string_equals_ignore_case(TAKE,instruccion)){

		log_info(logs,"entre al take");

		int32_t auxiliar_numero = 4;
		enviar_parametros(program_counter,auxiliar_numero);

		t_contenido mensaje_para_recibir_numero;
		memset(mensaje_para_recibir_numero,0,sizeof(t_contenido));
		t_header header_recibir_numero = recibirMensaje(socketMSP,mensaje_para_recibir_numero,logs);

		if(header_recibir_numero == MSP_TO_CPU_BYTES_ENVIADOS){

			program_counter = aumentarProgramCounter(program_counter,4);

		}

		int32_t auxiliar_registro = 1;
		enviar_parametros(program_counter,auxiliar_registro);

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

							}

				program_counter = aumentarProgramCounter(program_counter,1);
				free(registro);

			}
	}
	//#21 NOPP
	//Consume un ciclo del CPU sin hacer nada

	if(string_equals_ignore_case(NOPP,instruccion)){

		log_info(logs,"Consumo un Quantum");

	}

	//#25 MALC

	if(string_equals_ignore_case(MALC,instruccion)){

		log_info(logs,"El MALC esta por entrar");


		instruccion_MALC();
	}

	//#26 FREE

	if(string_equals_ignore_case(FREE,instruccion)){

				log_info(logs,"El FREE esta por entrar");

				instruccion_FREE();

	}

	//#27 INNN

	if(string_equals_ignore_case(INNN,instruccion)){

				log_info(logs,"El INNN esta por entrar");

				instruccion_INNN();

	}


	//#24 XXXX

	if(string_equals_ignore_case(XXXX,instruccion)){


		log_info(logs,"El XXXX esta por entrar");

		instruccion_XXXX();

		cont = QUANTUM;
		termino_proceso_XXXX = 1;

	}

	//#28 INNC

	if(string_equals_ignore_case(INNC,instruccion)){

		log_info(logs,"El INNC esta por entrar");

		instruccion_INNC();

	}

	//#31 CREA
	if(string_equals_ignore_case(CREA,instruccion)){

		log_info(logs,"El CREA esta por entrar");

		instruccion_CREA();


	}

	//#29 OUTN
	if(string_equals_ignore_case(OUTN,instruccion)){

		log_info(logs,"El OUTN esta por entrar");

		instruccion_OUTN();
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

	return -1;
}

void funcion_verificador_segundo_registro_GETM(int32_t *var,int32_t aux2){

					switch(aux2){
							case 0 : instruccion_GETM(var,&A);
								break;
							case 1 :
								instruccion_GETM(var,&B);
								break;
							case 2 : instruccion_GETM(var,&C);
								break;
							case 3 : instruccion_GETM(var,&D);
								break;
							case 4 : instruccion_GETM(var,&E);

						}
					}



void enviar_parametros(int32_t program_counter,int32_t auxiliar){

	t_contenido mensaje_cantidad_bytes;
	memset(mensaje_cantidad_bytes,0,sizeof(t_contenido));
	strcpy(mensaje_cantidad_bytes, string_from_format("[%d,%d]",program_counter,auxiliar));
	enviarMensaje(socketMSP,CPU_TO_MSP_PARAMETROS,mensaje_cantidad_bytes,logs);
}

