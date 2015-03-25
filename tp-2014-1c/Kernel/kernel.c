/*
 * Sistemas Operativos - Esta Coverflow.
 * Grupo       : The codes remains the same.
 * Nombre      : Kernel.c.
 * Descripcion : Este archivo contiene la implementacion de las funciones
 * utilizadas por el kernel.
 */

#include "kernel.h"
extern t_log* kernelLog;
extern t_log* queueLog;

extern pthread_cond_t mutexes[]= {
	    PTHREAD_COND_INITIALIZER,
	    PTHREAD_COND_INITIALIZER,
	    PTHREAD_COND_INITIALIZER,
	    PTHREAD_COND_INITIALIZER,
	    PTHREAD_COND_INITIALIZER,
	    PTHREAD_COND_INITIALIZER,
	    PTHREAD_COND_INITIALIZER,
	    PTHREAD_COND_INITIALIZER
	};

int main(int argc, char* argv[]) {

	//Verifica si se recibio como parametro el path del config, sino aborta la ejecucion
	if (argc == 1) {
		printf("Error, falta el path al punto de montaje.\n");
		exit(EXIT_FAILURE);
	}

	//Crea un archivo de log para el kernel
	kernelLog = log_create(KernelLogPath, "Kernel", true, LOG_LEVEL_DEBUG);
	//Crea un archivo para log de colas
	queueLog = log_create(QueueLogPath, "Kernel - Queues", false, LOG_LEVEL_INFO);

	// Hello Kernel!
	system("clear");
	int kernel_pid = getpid();
	log_info(kernelLog, "************** WELCOME TO KERNEL V1.0! (PID: %d) ***************\n", kernel_pid);

	StartKernel(argv[1]);

	pthread_create(&plpThread.tid, NULL, (void*) plp, (void*) &plpThread);
	pthread_create(&pcpThread.tid, NULL, (void*) pcp, (void*) &pcpThread);
	pthread_create(&readyQueueManagerThread.tid, NULL, (void*) exit_queue_manager, &readyQueueManagerThread );
	pthread_create(&exitQueueManagerThread.tid, NULL, (void*) ready_queue_manager, &exitQueueManagerThread );

	pthread_join(plpThread.tid, NULL ); //espera que finalice el hilo para continuar
	pthread_join(pcpThread.tid, NULL ); //espera que finalice el hilo para continuar
	pthread_join(readyQueueManagerThread.tid, NULL ); //espera que finalice el hilo para continuar
	pthread_join(exitQueueManagerThread.tid, NULL ); //espera que finalice el hilo para continuar


	return EXIT_SUCCESS;
}

/**
 * @NAME: StartKernel
 * @DESC: Realiza tareas de inicializacion del kernel
 */
void StartKernel(char* configPath) {
	/*Inicializa Properties*/

	//carga el archivo de configuracion y setea properties del kernel.
	LoadConfig(configPath);
	PUERTO = config_kernel.SELF_P;
	NO_SEMAPHORE = string_new();
	NO_SEMAPHORE = string_duplicate("NO_SEMAPHORE");

	NO_IO = string_new();
	NO_IO = string_duplicate("NO_IO");

	//Inicializa lista de Cpu's
	cpu_client_list = list_create();

	pthread_mutex_init(&mutex_cpu_list, NULL);

	//Inicializa colas
	new_queue = queue_create();
	ready_queue = queue_create();
	block_queue = queue_create();
	exec_queue = queue_create();
	exit_queue = queue_create();

	//Inicializa semaforo de colas
	pthread_mutex_init(&mutex_new_queue, NULL );
	pthread_mutex_init(&mutex_ready_queue, NULL );
	pthread_mutex_init(&mutex_block_queue, NULL );
	pthread_mutex_init(&mutex_exec_queue, NULL );
	pthread_mutex_init(&mutex_exit_queue, NULL );

	pthread_cond_init(&cond_ready_consumer, NULL ); /* Initialize consumer condition variable */
	pthread_cond_init(&cond_ready_producer, NULL ); /* Initialize consumer condition variable */
	pthread_cond_init(&cond_exit_consumer, NULL ); /* Initialize consumer condition variable */
	pthread_cond_init(&cond_exit_producer, NULL ); /* Initialize consumer condition variable */
	pthread_cond_init(&condpBlockedProcess, NULL ); /* Initialize producer condition variable */


	//Inicializar hilos de I/O
	char** IO_ID_array = string_get_string_as_array(config_kernel.IO_ID);
	char** IO_R_array = string_get_string_as_array(config_kernel.IO_RETARDO);

	log_info(kernelLog, string_from_format( "El grado de multiprogramacion del Kernel es: %i\n", config_kernel.MULTIPROG));

	int i = 0;

	//Si hay dispositivos de entrada salida...me pongo a trabajar (?)
	if(IO_ID_array[i] != NULL){

		//Genero una lista para almacenar las estructuras de los dispositivos en el Kernel
		ioList = list_create();

		log_info(kernelLog, "*****************Creando hilos I/O*******************");
		while (IO_ID_array[i] != NULL ) {
			log_debug(kernelLog, "Se agrego nuevo dispositivo: %s | Retardo: %i", IO_ID_array[i], atoi(IO_R_array[i]));

			t_iothread* ioThread = calloc(1, sizeof(t_iothread));

				ioThread->nombre = string_new();
				ioThread->nombre = string_duplicate(IO_ID_array[i]);
				ioThread->retardo = (int32_t)atoi(IO_R_array[i]);
				pthread_cond_init(&mutexes[i], NULL );

				ioThread->mutexes_consumer = i;

			pthread_create (&ioThread->tid, NULL, (void*)io, ((t_iothread*)ioThread));

			list_add(ioList, ioThread);
			i++;

			usleep(1000000);
		}
	}
	else{
		log_info(kernelLog, "No se cargaron dispositivos IO en el archivo de configuración!");
	}
	log_info(kernelLog, "\n");

	free(IO_ID_array);
	free(IO_R_array);

	/************************************Inicializa diccionario de semaforos.*************************************/

	semaforosAnsisop_list = list_create();

	int32_t semaphoreNumber = 0;
	char** SEMAPHORE_VALUE_array = string_get_string_as_array(config_kernel.VALOR_SEMAFOROS);
	char** SEMAPHORE_NAME_array = string_get_string_as_array(config_kernel.SEMAFOROS);

	log_info(kernelLog, "*****************Creando Semaforos de Kernel*******************");
	while (SEMAPHORE_NAME_array[semaphoreNumber] != NULL ) {
		t_semaforoAnsisop* semaforo = malloc(sizeof(t_semaforoAnsisop));

		memset(semaforo->Id, 0, sizeof(semaforo->Id));
		strcpy(semaforo->Id, SEMAPHORE_NAME_array[semaphoreNumber]);

		semaforo->Valor = atoi(SEMAPHORE_VALUE_array[semaphoreNumber]);

		list_add(semaforosAnsisop_list, semaforo);

		semaphoreNumber++;
	}

	log_info(kernelLog, "Semáforos creados: ");

	for (semaphoreNumber = 0; semaphoreNumber < list_size(semaforosAnsisop_list); semaphoreNumber++) {
		log_info(kernelLog, string_from_format("-- ID= %s.......VALUE= %d", ((t_semaforoAnsisop*)list_get(semaforosAnsisop_list, semaphoreNumber))->Id
																		 ,((t_semaforoAnsisop*)list_get(semaforosAnsisop_list, semaphoreNumber))->Valor));
	}

	log_info(kernelLog, "\n");

	free(SEMAPHORE_VALUE_array);
	free(SEMAPHORE_NAME_array);

	/*****************************Inicializa lista de variables compartidas (Globales)********************/

	var_compartida_list = list_create();

	int32_t variablesCounter = 0;
	char** VARIABLE_ID_array = string_get_string_as_array(config_kernel.ID_VARIABLES);
	char** VARIABLE_VALOR_array = string_get_string_as_array(config_kernel.VALOR_VARIABLES);

	log_info(kernelLog, "****Cargando variables compartidas del Kernel***");
	while (VARIABLE_ID_array[variablesCounter] != NULL ) {
		t_var_compartida* variableCompartida = malloc(sizeof(t_var_compartida));
		memset(variableCompartida->Id, 0, sizeof(variableCompartida->Id));
		strcpy(variableCompartida->Id, VARIABLE_ID_array[variablesCounter]);
		variableCompartida->Valor = atoi(VARIABLE_VALOR_array[variablesCounter]);

		list_add(var_compartida_list, variableCompartida);

		variablesCounter++;
	}

	log_info(kernelLog, "Variables cargadas con éxito!");

	for(variablesCounter = 0; variablesCounter < var_compartida_list->elements_count; variablesCounter++) {
		log_info(kernelLog,
				string_from_format("-- ID= %s.......VALUE= %d"
						,((t_var_compartida*)list_get(var_compartida_list, variablesCounter))->Id
						,((t_var_compartida*)list_get(var_compartida_list, variablesCounter))->Valor));
	}
	log_info(kernelLog, "\n");

	free(VARIABLE_ID_array);
	free(VARIABLE_VALOR_array);

	/*Se valida que en el sistema exista una instancia de la UMV levantada. Esto es indispensable
	 * para lograr reservar segmentos para posibles clientes programas*/
	 socketUMV = conectarAServidor(config_kernel.IP_UMV, config_kernel.PUERTO_UMV);

	 while (socketUMV == EXIT_FAILURE) {
		 log_info(kernelLog, "Despierten a la UMV! Se reintenta conexion en unos segundos ;) \n");
		 sleep(5);
		 socketUMV = conectarAServidor(config_kernel.IP_UMV, config_kernel.PUERTO_UMV);
	 }

	 HandshakeUMV();

	log_info(kernelLog, "Se ha establecido conexion con el proceso UMV\n");
}

/**
 * @NAME: LoadConfig
 * @DESC: Carga las variables con valores del archivo de configuracion.
 */
void LoadConfig(char* configPath) {

	log_info(kernelLog, "Se inicializa el kernel con parametros desde: %s",
			configPath);
	t_config* kernelConfig = config_create(configPath);

	if (config_has_property(kernelConfig, "PUERTO_PROG")) {
		//strcpy(PUERTO_PROG, (char*) config_get_string_value(configPath, "PUERTO_PROG"));
		config_kernel.PUERTO_PROG = config_get_int_value(kernelConfig,
				"PUERTO_PROG");
	} else {
		log_error(kernelLog,
				"No se encontro la key 'PUERTO_PROG' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "STACK_SIZE")) {
		//strcpy(PUERTO_PROG, (char*) config_get_string_value(configPath, "PUERTO_PROG"));
		config_kernel.STACK_SIZE = config_get_int_value(kernelConfig,
				"STACK_SIZE");
	} else {
		log_error(kernelLog,
				"No se encontro la key 'STACK_SIZE' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "PUERTO_CPU")) {
		//strcpy(PUERTO_CPU, (char*) config_get_string_value(configPath, "PUERTO_CPU"));
		config_kernel.PUERTO_CPU = config_get_int_value(kernelConfig,
				"PUERTO_CPU");
	} else {
		log_error(kernelLog,
				"No se encontro la key 'PUERTO_CPU' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (!config_has_property(kernelConfig, "IPYPUERTOUMV")) {
		log_error(kernelLog,
				"No se encontro la key 'IPYPUERTOUMV' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "QUANTUM")) {
		config_kernel.QUANTUM = config_get_int_value(kernelConfig, "QUANTUM");
	} else {
		log_error(kernelLog,
				"No se encontro la key 'QUANTUM' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "RETARDO")) {
		config_kernel.RETARDO_QUANTUM = config_get_int_value(kernelConfig,
				"RETARDO");
	} else {
		log_error(kernelLog,
				"No se encontro la key 'RETARDO' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "MULTIPROGRAMACION")) {
		config_kernel.MULTIPROG = config_get_int_value(kernelConfig,
				"MULTIPROGRAMACION");
	} else {
		log_error(kernelLog,
				"No se encontro la key 'MULTIPROGRAMACION' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}
	if (config_has_property(kernelConfig, "SELF_P")) {
		config_kernel.SELF_P = config_get_int_value(kernelConfig, "SELF_P");
	} else {
		log_error(kernelLog,
				"No se encontro la key 'SELF_P' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "SEMAFOROS")) {
		config_kernel.SEMAFOROS = string_duplicate(
				config_get_string_value(kernelConfig, "SEMAFOROS"));
	} else {
		log_error(kernelLog,
				"No se encontro la key 'SEMAFOROS' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "VALOR_SEMAFORO")) {
		config_kernel.VALOR_SEMAFOROS = string_duplicate(
				config_get_string_value(kernelConfig, "VALOR_SEMAFORO"));
	} else {
		log_error(kernelLog,
				"No se encontro la key 'VALOR_SEMAFORO' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}
	if (config_has_property(kernelConfig, "ID_VARIABLES")) {
		config_kernel.ID_VARIABLES = string_duplicate(
				config_get_string_value(kernelConfig, "ID_VARIABLES"));
	} else {
		log_error(kernelLog,
				"No se encontro la key 'ID_VARIABLES' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "VALOR_VARIABLES")) {
		config_kernel.VALOR_VARIABLES = string_duplicate(
				config_get_string_value(kernelConfig, "VALOR_VARIABLES"));
	} else {
		log_error(kernelLog,
				"No se encontro la key 'VALOR_VARIABLES' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}
	if (config_has_property(kernelConfig, "HIO")) {
		config_kernel.IO_RETARDO = string_duplicate(
				config_get_string_value(kernelConfig, "HIO"));
	} else {
		log_error(kernelLog,
				"No se encontro la key 'HIO' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	if (config_has_property(kernelConfig, "ID_HIO")) {
		config_kernel.IO_ID = string_duplicate(
				config_get_string_value(kernelConfig, "ID_HIO"));
	} else {
		log_error(kernelLog,
				"No se encontro la key 'ID_HIO' en el archivo de configuracion");
		config_destroy(kernelConfig);
		exit(EXIT_FAILURE);
	}

	char* umv = (char*) malloc(sizeof(char) * 30);
	umv = string_duplicate(
			config_get_string_value(kernelConfig, "IPYPUERTOUMV"));
	char** arrayIPyP = string_split(umv, ":");
	config_kernel.IP_UMV = string_duplicate(arrayIPyP[0]);
	config_kernel.PUERTO_UMV = atoi(arrayIPyP[1]);

	log_info(kernelLog, "Se termino de cargar los datos del Kernel");

	free(umv);
	free(arrayIPyP);
	config_destroy(kernelConfig);

}

void ComunicarMuertePrograma(int32_t processFd, bool wasInUmv) {

	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));

	log_info(kernelLog,
			string_from_format(
					"Se informa al programa con Fd: %d, que su ejecución finalizó",
					processFd));
	if(wasInUmv){//Si el proceso no estaba en la UMV entonces se rechazó por falta de memoria en el sistema!
	strcpy(mensaje, string_from_format("I believe that your time has come, fd %d!", processFd));
	enviarMensaje(processFd, KRN_TO_PRG_END_PRG, mensaje, kernelLog);
	}
	else{
		strcpy(mensaje, "Sorry, we cannot process you right now, so try again later!");
		enviarMensaje(processFd, KRN_TO_PRG_NO_MEMORY, mensaje, kernelLog);
	}

}

void HandshakeUMV() {

	t_contenido mensaje;
	enviarMensaje(socketUMV, KRN_TO_UMV_HANDHAKE, mensaje, kernelLog);

}

void EliminarSegmentos(int32_t pID) {

	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("%d", pID));

	if (enviarMensaje(socketUMV, KRN_TO_UMV_ELIMINAR_SEGMENTOS, mensaje, kernelLog) == EXIT_FAILURE) {
		log_info(kernelLog, "Error al enviar mensaje a UMV, solicitando remover segmentos de memoria de un proceso! :/");
	} else {
		log_info(kernelLog, string_from_format("Se solicitó a la UMV, remover los segmentos correspondientes al proceso PID: %d", pID));
	}

}

bool SolicitarSegmentosUMV(t_process* aProcess) {

	t_medatada_program* structData = metadata_desde_literal(aProcess->scriptCode);
	t_contenido mensaje;
	int32_t pID = aProcess->process_pcb->pId;


	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("[%d, %d]", pID, config_kernel.STACK_SIZE));

	log_info(kernelLog, string_from_format("solicitud de stack...se envia: %s", mensaje));
	enviarMensaje(socketUMV, KRN_TO_UMV_MEM_REQ, mensaje, kernelLog);

	memset(mensaje, 0, sizeof(t_contenido));
	t_header header = recibirMensaje(socketUMV, mensaje, kernelLog);
	if(header == UMV_TO_KRN_MEMORY_OVERLOAD){
		log_info(kernelLog, "La UMV me informa que no existe memoria disponible para alojar otro programa cuando solicité espacio para el Stack :/");

		return false;
	}
	else{
			aProcess->process_pcb->segmentoStack = atoi(mensaje);
			aProcess->process_pcb->cursorStack = atoi(mensaje);
			int32_t pID = aProcess->process_pcb->pId;
			int32_t tamanio = strlen(aProcess->scriptCode);

			memset(mensaje, 0, sizeof(t_contenido));
			strcpy(mensaje, string_from_format("[%d, %d]", pID, tamanio));
			enviarMensaje(socketUMV, KRN_TO_UMV_MEM_REQ, mensaje, kernelLog);
			log_info(kernelLog, string_from_format("solicitud por tamanio de proceso...se envia: %s", mensaje));

			memset(mensaje, 0, sizeof(t_contenido));
			t_header header = recibirMensaje(socketUMV, mensaje, kernelLog);
			if(header == UMV_TO_KRN_MEMORY_OVERLOAD){
				log_info(kernelLog, "La UMV me informa que no existe memoria disponible para alojar otro programa cuando solicité el Segmento de código! :/ ... ");

				EliminarSegmentos(pID);
				return false;
			}
			else{
				aProcess->process_pcb->segmentoCodigo = atoi(mensaje);
				int32_t pID = aProcess->process_pcb->pId;
				int32_t tamanio = ((structData->instrucciones_size) * 8);

				memset(mensaje, 0, sizeof(t_contenido));
				strcpy(mensaje, string_from_format("[%d, %d]", pID, tamanio));
				enviarMensaje(socketUMV, KRN_TO_UMV_MEM_REQ, mensaje, kernelLog);
				log_info(kernelLog, string_from_format("solicitud indice de codigo...se envia: %s", mensaje));

				memset(mensaje, 0, sizeof(t_contenido));
				t_header header = recibirMensaje(socketUMV,mensaje, kernelLog);
				if(header == UMV_TO_KRN_MEMORY_OVERLOAD){
					log_info(kernelLog, "La UMV me informa que no existe memoria disponible para alojar otro programa cuando solicité el Segmento para el indice de código! :/ ...");
					EliminarSegmentos(pID);
					return false;

				}
				else{
					aProcess->process_pcb->indiceCodigo = atoi(mensaje);
					int32_t pID = aProcess->process_pcb->pId;
					int32_t tamanio = structData->etiquetas_size;
					if(tamanio > 0){

						memset(mensaje, 0, sizeof(t_contenido));
						strcpy(mensaje, string_from_format("[%d, %d]", pID, tamanio));
						enviarMensaje(socketUMV, KRN_TO_UMV_MEM_REQ, mensaje, kernelLog);
						log_info(kernelLog, string_from_format("solicitud de segmento de etiquetas...se envia: %s", mensaje));

						memset(mensaje, 0, sizeof(t_contenido));
						t_header header = recibirMensaje(socketUMV,mensaje, kernelLog);
						if(header == UMV_TO_KRN_MEMORY_OVERLOAD){
							log_info(kernelLog, "La UMV me informa que no existe memoria disponible para alojar otro programa cuando le hablé sobre el Segmento de etiquetas. Expulso al programa :/ ... ! ");
							EliminarSegmentos(pID);
							return false;
						}
						else{
							aProcess->process_pcb->indiceEtiquetas = atoi(mensaje);
						}
					}
					else{
						log_info(kernelLog, "El codigo no tiene etiquetas, no se reserva segmento para las etiquetas");
						return true;
					}
				}
			}
		}

	metadata_destruir(structData);

	return true;
}

bool EscribirSegmentosUMV(t_process* aProcess){

	t_medatada_program* structData = metadata_desde_literal(aProcess->scriptCode);
	t_contenido mensaje;

		char* codigo = string_new();
		codigo = string_duplicate(aProcess->scriptCode);
		//strcpy(codigo, aProcess->process_pcb->segmentoCodigo);

		char* buffer = string_new();		//Guardo el texto a medida que voy leyendo
		int32_t position = 0;	//Posicion BASE
		int32_t offset = 0;		//Desplazamiento dentro de mi buffer (seek)

		while(offset < strlen(codigo)){


			string_append(&buffer, string_from_format("%c", codigo[offset]));
			if(codigo[offset] == '\n'){
				memset(mensaje, 0, sizeof(t_contenido));
				strcpy(mensaje, string_from_format("[%d,%d,%s]", aProcess->process_pcb->segmentoCodigo, position, string_substring(codigo, position, offset + 1 - position)));

				enviarMensaje(socketUMV, KRN_TO_UMV_ENVIAR_BYTES, mensaje, kernelLog);
				recibirMensaje(socketUMV,mensaje, kernelLog);
				buffer = string_new();
				position = offset + 1;
			}

			offset ++;
		}

	char* instruccionSerialized = string_new();

	int32_t instruccionesCounter = 0;
	int32_t bytesOffset = 0;
	int32_t bytesTamanio = 8;
	for(instruccionesCounter = 0; instruccionesCounter < structData->instrucciones_size; instruccionesCounter++){

		int32_t start = structData->instrucciones_serializado[instruccionesCounter].start;
		char* filledStart = FillNumberWithZero(string_from_format("%d", start), 4);

		int32_t offset = structData->instrucciones_serializado[instruccionesCounter].offset;
		char* filledOffset = FillNumberWithZero(string_from_format("%d", offset), 4);

		string_append(&instruccionSerialized, filledStart);
		string_append(&instruccionSerialized, filledOffset);

		t_contenido mensaje;
		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d,%d,%s%s]", aProcess->process_pcb->indiceCodigo, bytesOffset, filledStart, filledOffset));
		log_info(kernelLog, mensaje);
		enviarMensaje(socketUMV, KRN_TO_UMV_ENVIAR_BYTES, mensaje, kernelLog);
		recibirMensaje(socketUMV,mensaje, kernelLog);
		memset(instruccionSerialized, 0, sizeof(instruccionSerialized));
		bytesOffset += bytesTamanio;
	}


	/*Se pide a UMV escribir etiquetas*/
	if(aProcess->process_pcb->indiceEtiquetas_size > 0){
		t_contenido mensaje;

		log_debug(kernelLog, "Verifico la longitud de la cadena de etiquetas para no sobrepasar el buffer de mi comunicación!");
		int32_t strlength = strlen(structData->etiquetas);

		log_debug(kernelLog, string_from_format("La longitud del serializado es %d", strlength));
		log_debug(kernelLog, string_from_format("El etiquetas size es %d", structData->etiquetas_size));


		if(strlength < 256){

			char* etiquetas = (char*)malloc(sizeof(char)*structData->etiquetas_size);
			memcpy(etiquetas, structData->etiquetas, structData->etiquetas_size);
			memset(mensaje, 0, sizeof(t_contenido));

			char* bufferTmp = string_new();

			int k = 0;

			while(k < structData->etiquetas_size){
				string_append(&bufferTmp, string_from_format("%c", etiquetas[k]));
				k++;
			}


			string_append(&bufferTmp, "]");

			strcpy(mensaje, string_from_format("%d", aProcess->process_pcb->indiceEtiquetas));
			enviarMensaje(socketUMV, KRN_TO_UMV_ENVIAR_ETIQUETAS, mensaje, kernelLog);
			recibirMensaje(socketUMV, mensaje, kernelLog);
			send(socketUMV, etiquetas, structData->etiquetas_size, 0);

			recibirMensaje(socketUMV,mensaje, kernelLog);
		}
		else{

			log_debug(kernelLog, "Al parecer no entra todo en un mensaje! :/");

			int32_t msg_size_tmp = 0;
			int32_t msg_offset_tmp = 200;

			while(msg_size_tmp < strlen(mensaje)){

				memset(mensaje, 0, sizeof(t_contenido));
				//bufferTemp = string_duplicate(string_substring(structData->etiquetas, msg_size_tmp, msg_offset_tmp));
				strcpy(mensaje, string_duplicate(string_substring(mensaje, msg_size_tmp, msg_offset_tmp)));
				enviarMensaje(socketUMV, KRN_TO_UMV_ENVIAR_BYTES, string_from_format("[%d,%d,%s]", aProcess->process_pcb->indiceEtiquetas, bytesOffset, mensaje), kernelLog);
				recibirMensaje(socketUMV,mensaje, kernelLog);
				msg_size_tmp += msg_offset_tmp;
			}
		}

	}
	else{
		log_info(kernelLog, "No había etiquetas...se omite la escritura para este dato");
	}

	//libero memoria! :) I'M FEEL FREEEEEEEE!
	metadata_destruir(structData);

	return true;
}


t_iothread* GetIOThreadByName(char* ioName){

	bool _match_IO_ID(void* element) {
			//((t_process*)element)->blockedBySemaphore == NO_SEMAPHORE
				if (string_equals_ignore_case(((t_iothread*)element)->nombre, ioName)) {
					return true;
				}
				return false;
			}

		return (t_iothread*)list_find(ioList, (void*)_match_IO_ID);

		//return list_any_satisfy(aList, (void*)_blocked_by_IO);
}

/**
 * @NAME: KillProcess
 * @DESC: Elimina un proceso del sistema y toda la memoria que solicitó.
 */
void KillProcess(t_process* aProcess){

	if(StillInside(aProcess->process_fd)){
		log_info(kernelLog, string_from_format("Se elimina del sistema las estructuras asociadas al proceso con PID: %d", aProcess->process_pcb->pId));
		free(aProcess->process_pcb);
		free(aProcess->scriptCode);
		free(aProcess);
	}
	else{
		aProcess->process_fd = 0;
	}

}

bool _match_pid(void* element){
	//TODO Reveer esto!
	if(((t_process*)element)->process_pcb->pId == 0){
		return true;
	}
	return false;
}

void enviarAEjecutar(int32_t socketCPU, int32_t  quantum, t_process* aProcess){

		int32_t v1 = aProcess->process_pcb->pId;
		int32_t v2 = aProcess->process_pcb->contextoActual_size;
		int32_t v3 = aProcess->process_pcb->programCounter;
		int32_t v4 = aProcess->process_pcb->indiceEtiquetas_size;
		int32_t v5 = aProcess->process_pcb->indiceEtiquetas;
		int32_t v6 = aProcess->process_pcb->indiceCodigo;
		int32_t v7 = aProcess->process_pcb->segmentoCodigo;
		int32_t v8 = aProcess->process_pcb->cursorStack;
		int32_t v9 = aProcess->process_pcb->segmentoStack;

		t_contenido mensaje;
		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]", v1, v2, v3, v4, v5, v6, v7, v8, v9, config_kernel.QUANTUM, config_kernel.RETARDO_QUANTUM));
		enviarMensaje(socketCPU, KRN_TO_CPU_PCB, mensaje, kernelLog);
		log_info(kernelLog, "Se envía un PCB al CPU libre elegido");
//		addExecProcess();
}

char* FillNumberWithZero(char* number, int32_t fullSize){
	char* response = string_new();

	int32_t zeros = fullSize - strlen(number);
	int32_t counter;
	for(counter = 0; counter < zeros; counter++){
		string_append(&response, "0");
	}

	string_append(&response, number);


	return response;
}

bool _cpuIsFree(void* element){

		if(((t_client_cpu*)element)->isBusy == false){
			return true;
		}
		return false;
	}

t_client_cpu* GetCPUReady(){
	return list_find(cpu_client_list, (void*)_cpuIsFree);
}

/*
 * Ojo con este metodo...no extrae ni modifica el estado de un CPU.
 * SOLO UTILIZAR EN CASO DE QUE SE SEPA QUE SIGUE EN EL SISTEMA. (PCP - FIN DE QUANTUM)
 */
t_client_cpu* GetCPUByCPUFd(int32_t cpuFd){

	bool _match_cpu_fd(void* element){
		if(((t_client_cpu*)element)->cpuFD == cpuFd){
			return true;
		}
		return false;
	}

	return list_find(cpu_client_list, (void*)_match_cpu_fd);
}
