/*
 ============================================================================
 Name        : Cpu.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : CPU, Ansi-style
 ============================================================================
 */

#include "Cpu.h"
#include "Primitivas.h"

extern t_log * logCpu;


extern int32_t socketUMV;
extern int32_t socketKernel;
extern int32_t quantum;
extern t_pcb* aPCB;
extern bool endFlag;
extern t_dictionary *variablesAnsisop;
bool continuar;
extern bool contextSwitch;
extern bool bloqueado;
extern bool conError;

/*Flag para evaluar si el CPU tiene algún script corriendo o puede finalizar directamente*/
bool someoneRunning = false;

int main(void) {

	int32_t myPID = process_getpid ();

	//cuando recibe una SIGNAL, la cambia a 'false' para cerrar CPU al finalizar el quantum
	continuar = true;
	contextSwitch = false;
	int32_t quantumValidator = 0;
	int32_t retardoQuantum = 0;
	quantum = 0;

	logCpu = log_create("cpu.log", "CPU", true, LOG_LEVEL_DEBUG);
	log_info(logCpu, "Se inicializa CPU");

	// levanto configuración
	t_config * config     = config_create("cpu.config");
	char* ipKernel        = config_get_string_value(config, "ipKernel");
	char* ipUMV           = config_get_string_value(config, "ipUMV");
	int32_t puertoKernel  = config_get_int_value(config, "puertoKernel");
	int32_t puertoUMV     = config_get_int_value(config, "puertoUMV");

	// signals
	signal(SIGINT, rutinasSeniales);
	signal(SIGTERM, rutinasSeniales);
	signal(SIGUSR1, rutinasSeniales);

	//creo diccionario de variables ansisop
	variablesAnsisop = dictionary_create();

	// me conecto al kernel y memoria
	log_info(logCpu, "Conexion a kernel ip:%s y puerto:%d", ipKernel, puertoKernel);
	log_info(logCpu, "Conexion a UMV ip:%s y puerto:%d", ipUMV, puertoUMV);

	//conexion a KERNEL
	socketKernel = conectarAServidor(ipKernel, puertoKernel);
	while (socketKernel == EXIT_FAILURE) {
		log_info(logCpu,
				"Despierten al Kernel! Se reintenta conexion en unos segundos ;) \n");
		sleep(5);
		socketKernel = conectarAServidor(ipKernel, puertoKernel);
	}

	//hago el handshake
	usleep(500000);
	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("%d", myPID));
	enviarMensaje(socketKernel, CPU_TO_KRN_HANDSHAKE, mensaje, logCpu);

	//conexion a UMV
	socketUMV = conectarAServidor(ipUMV, puertoUMV);
	while (socketUMV == EXIT_FAILURE) {
		log_info(logCpu,
				"Despierten a la UMV! Se reintenta conexion en unos segundos ;) \n");
		sleep(5);
		socketUMV = conectarAServidor(ipUMV, puertoUMV);
	}

	//hago el handshake
	usleep(500000);
	enviarMensaje(socketUMV, CPU_TO_UMV_HANDSHAKE, "", logCpu);

	while(continuar){

		t_contenido mensaje;
		memset(mensaje, 0, sizeof(t_contenido));

		t_header header = recibirMensaje(socketKernel, mensaje, logCpu);

		if(header == ERR_CONEXION_CERRADA){
			log_info(logCpu, "Se cerró la conexion con kernel");
			exit(EXIT_FAILURE);
		}
		else if(header == KRN_TO_CPU_PCB){

			someoneRunning = true;

			//Recibo PCB nuevo! :D
			log_info(logCpu, "recibir el PCB");
			
			char** pcbArray = string_get_string_as_array(mensaje);
			aPCB = malloc(sizeof(t_pcb));

			aPCB->pId                  = atoi(pcbArray[0]);
			aPCB->contextoActual_size  = atoi(pcbArray[1]);
			aPCB->programCounter       = atoi(pcbArray[2]);
			aPCB->indiceEtiquetas_size = atoi(pcbArray[3]);
			aPCB->indiceEtiquetas      = atoi(pcbArray[4]);
			aPCB->indiceCodigo         = atoi(pcbArray[5]);
			aPCB->segmentoCodigo       = atoi(pcbArray[6]);
			aPCB->cursorStack          = atoi(pcbArray[7]);
			aPCB->segmentoStack        = atoi(pcbArray[8]);

			bloqueado = false;
			conError = false;

			quantum = atoi(pcbArray[9]);
			retardoQuantum = (atoi(pcbArray[10]))*1000;

			int32_t tamCA;
			tamCA = aPCB->contextoActual_size;
			int32_t offset = (aPCB->cursorStack - aPCB->segmentoStack) - (tamCA * 5);
			int32_t dirVar;
			int32_t i;
	

			memset(mensaje, 0, sizeof(t_contenido));
			strcpy(mensaje, string_from_format("[%d,%d,%d]",aPCB->segmentoStack,offset,tamCA * 5));
			enviarMensaje(socketUMV,CPU_TO_UMV_SOLICITAR_BYTES, mensaje,logCpu);
			log_info(logCpu, "Se envía mensaje a UMV para recuperar el diccionario del contexto actual");
			memset(mensaje, 0, sizeof(t_contenido));
			recibirMensaje(socketUMV, mensaje,logCpu);
			
			log_info(logCpu, "Se limpia el diccionario para comenzar a ejecutar un Proceso Nuevo");
			dictionary_clean(variablesAnsisop);
			char* string = string_new();
			string = string_duplicate(mensaje);
			int32_t start = 1;
			int32_t x = 0;

			for(i = 0;i<tamCA;i++){

				//id = ;
				dirVar = offset + (i*5);
				dictionary_put(variablesAnsisop, string_duplicate(string_substring(string, x, 1)), (void*)(dirVar));
				log_info(logCpu, string_from_format("Se agrega la variable ID: %s Valor: %d", string_duplicate(string_substring(string, x, 1)), (void*)(dirVar)));
				x = x + 5;
				start = start + 5;
			}
				
			
			
			
			endFlag = false;
			quantumValidator = quantum;

			//Empiezo a iterar por quantums
			while (quantum > 0) {



				// pedimos la instruccion a la UMV
				memset(mensaje, 0, sizeof(t_contenido));
				strcpy(mensaje,string_from_format("[%d,%d,%d]", aPCB->indiceCodigo, aPCB->programCounter *8, 8));
				enviarMensaje(socketUMV, CPU_TO_UMV_SOLICITAR_BYTES, mensaje, logCpu);

				memset(mensaje, 0, sizeof(t_contenido));
				header = recibirMensaje(socketUMV, mensaje, logCpu);

				//Valido que no se haya caido la conexión
				if(header == ERR_CONEXION_CERRADA){
					log_info(logCpu, "Se cerró la conexion con la UMV");
					exit(EXIT_FAILURE);
				}
				else{

					//Recibo el indice de la instruccion a ejecutar
					char* start = string_new();
					char* offset= string_new();

					strcpy(start, string_substring(mensaje, 0, 4));
					strcpy(offset, string_substring(mensaje, 4, 4));

					//Le pido la sentencia a ejecutar
					memset(mensaje, 0, sizeof(t_contenido));
					strcpy(mensaje,string_from_format("[%d,%s,%s]", aPCB->segmentoCodigo, start, offset));
					enviarMensaje(socketUMV, CPU_TO_UMV_SOLICITAR_BYTES, mensaje, logCpu);

					memset(mensaje, 0, sizeof(t_contenido));
					header = recibirMensaje(socketUMV, mensaje, logCpu);

					if(header == ERR_CONEXION_CERRADA){
						log_info(logCpu, "Se cerró la conexion con la UMV");
						exit(EXIT_FAILURE);
					}
					/*TODO: GITHUB ISSUE ASOCIADO : N°26
					else if(header == UMV_TO_CPU_SENTENCE) {
						log_info(logCpu, "UMV_TO_CPU_SENTENCE");
						ejecutarSentencia(mensaje);
					}
					else if(header == UMV_TO_CPU_SEGM_FAULT) {
						log_info(logCpu, "STACK OVERFLOW - Se cierra CPU");
						exit(EXIT_FAILURE);
					}
					else {
						log_warning(logCpu, "respuesta inesperada de la UMV a CPU");
					}*/

				}

				char* sentencia = string_new();
				sentencia = string_duplicate(mensaje);
				ejecutarSentencia(sentencia);
				if(quantum!=0){
				quantum--;
				}
				quantumValidator--;
				if(!contextSwitch){
					aPCB->programCounter ++;
				}
				else{
					contextSwitch = false;
				}

				log_info(logCpu, string_from_format("Se aguarda durante:%d milisegundos", (retardoQuantum/1000)));
				log_info(logCpu, string_from_format("QuantumValidator:%d y Quantum:%d", quantumValidator, quantum));
				usleep(retardoQuantum); //Retardo de quantum recibido.

			}//--END WHILE

			someoneRunning = false;

			if(quantumValidator == 0 && quantum == 0 && !endFlag && !bloqueado && !conError && continuar){ //Ejecuté toodo el quantum!?

				memset(mensaje, 0, sizeof(t_contenido));
				strcpy(mensaje, string_from_format("[%d,%d,%d,%d]", aPCB->pId, aPCB->programCounter, aPCB->contextoActual_size, aPCB->cursorStack));
				enviarMensaje(socketKernel, CPU_TO_KRN_END_PROC_QUANTUM, mensaje, logCpu);
			}
			else if(!continuar){
				memset(mensaje, 0, sizeof(t_contenido));
				strcpy(mensaje, string_from_format("[%d,%d,%d,%d]", aPCB->pId, aPCB->programCounter, aPCB->contextoActual_size, aPCB->cursorStack));
				enviarMensaje(socketKernel, CPU_TO_KRN_END_PROC_QUANTUM_SIGNAL, mensaje, logCpu);
			}

			if((quantumValidator != 0) && (quantum == 0) && !endFlag && !bloqueado && conError){
				log_info(logCpu, "::::::::::ERROR:::::::::: -----> Se sale por condicion de error en la ejecucion!");
				memset(mensaje, 0, sizeof(t_contenido));
				strcpy(mensaje, string_from_format("%d", aPCB->pId));
				enviarMensaje(socketKernel, CPU_TO_KRN_END_PROC_ERROR, mensaje, logCpu);
			}
			
			if((quantumValidator == 0) && (quantum == 0) && !endFlag && !bloqueado && conError){
				log_info(logCpu, "::::::::::ERROR:::::::::: -----> Se sale por condicion de error en la ejecucion!");
				memset(mensaje, 0, sizeof(t_contenido));
				strcpy(mensaje, string_from_format("%d", aPCB->pId));
				enviarMensaje(socketKernel, CPU_TO_KRN_END_PROC_ERROR, mensaje, logCpu);
			}

			if(!continuar){ //se sale porque alguien le mandó la señal
				log_info(logCpu, "CIERRO EL CPU POR UN SIGNAL");
				exit(EXIT_SUCCESS);
			}
		}
		else {
			log_warning(logCpu, "respuesta inesperada de KERNEL a CPU");
		}
	}
	return EXIT_SUCCESS;
}


/*
 * Rutinas de las señales.
 */
void rutinasSeniales(int senial){
	switch (senial) {
		case SIGTERM:
		case SIGUSR1:
		case SIGINT: // Interrupcion de programa: Control + c. Ejecutando 2 veces, mata el Cpu
			if(!someoneRunning){
				log_info(logCpu, "Se recibio la senial y nadie estaba corriendo por aca, cierro el CPU [FIN]");
				exit(0);
			}
			else if(!continuar) {
				log_info(logCpu, "Se recibio SIGINT por SEGUNDA vez, cierro el CPU [FIN]");
				exit(0);
			}
			else{
				log_info(logCpu, "Se recibio SIGINT, termino el CPU despues de concluir ejecucion de quantum");
				continuar = false;
			}
			break;
		default:
			log_warning(logCpu, "Se recibio una señal no manejada..");
			break;
	}
}



/*
 * Ejecuta una linea llamando al parser
 */
void ejecutarSentencia(char* sentencia){

	int32_t position = 0;

	char* buffer = string_new();
	while(position < strlen(sentencia)){

		if( sentencia[position] == '\n') {
			break;
		}
		string_append(&buffer, string_from_format("%c", sentencia[position]));
		position++;
	}
	
	log_debug(logCpu, "Ejecuto la sentencia: '%s'", buffer);

	string_trim(&sentencia);
	/*if(string_equals_ignore_case(buffer, "end")){
		log_info(logCpu, "Se encontró la línea de fin de programa END!");
		finalizar();
	}*/
	analizadorLinea(buffer, &functions, &kernel_functions);

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



