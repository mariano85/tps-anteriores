
// para debug mode
#define DEBUG true

#include "UMV.h"

// lista de segmentos de memoria (contiene los distintos bloque almacenados)
t_list * segmentos;

// tamaño de memoria principal
integer tamanioMem = 0;

// memoria principal donde se alojaran los segmentos
t_memoria memPpal;

//log
t_log* logUMV;
t_log* logUMVDump;

//algoritmo
integer algoritmo;

//retardo
integer retardo;

//t_config * config;
char* ip;
integer puerto;
integer stack;

integer baseValue;

bool running;

pthread_rwlock_t lock;

integer cantCpus;
integer cantCompactaciones;

int main(int argc, char* argv[]) {

	//Verifica si se recibio como parametro el path del config, sino aborta la ejecucion
	if (argc == 1) {
		printf("Error, falta el archivo de configuracion.\n");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL)); // para la primer direccion base virtual
	baseValue = (rand() % 20)+2; // para la primer direccion base virtual
	running = true; // si cambia a false, entonces termina de correr la memoria.
	cantCpus = 0;
	cantCompactaciones = 0;

	logUMV = log_create("umv.log", "UMV", true, LOG_LEVEL_DEBUG);
	logUMVDump = log_create("umvDump.log", "UMV", DEBUG, LOG_LEVEL_DEBUG);

	//inicio semaforos
	pthread_rwlock_init(&lock, NULL);

	log_info(logUMV, "Se inicializa UMV");
	log_info(logUMV, "la base del bloque inicial es: %d", baseValue);

	if(DEBUG){
		log_info(logUMV, "la base del bloque inicial 1 es: %d",getRandomBase());
		log_info(logUMV, "la base del bloque inicial 2 es: %d",getRandomBase());
		log_info(logUMV, "la base del bloque inicial 3 es: %d",getRandomBase());
		log_info(logUMV, "la base del bloque inicial 4 es: %d",getRandomBase());
		log_info(logUMV, "la base del bloque inicial 5 es: %d",getRandomBase());
	}

	// levanto configuración de "umv.config"
	t_config * config = config_create(argv[1]);
	ip = config_get_string_value(config, "ip");
	puerto = config_get_int_value(config, "puerto");
	stack = config_get_int_value(config, "stack");
	retardo = config_get_int_value(config, "retardo");
	algoritmo = config_get_int_value(config, "algoritmo");

	// stack += 1; // el +1 es para el '\0' del final

	log_info(logUMV, "ip: %s", ip);
	log_info(logUMV, "puerto: %d", puerto);
	log_info(logUMV, "stack: %d", stack);
	log_info(logUMV, "retardo: %d", retardo);
	log_info(logUMV, "algoritmo: %d", algoritmo);

	// lista de segmentos
	segmentos = list_create();

	// GRAN malloc
	memPpal = (t_memoria) malloc( (stack * sizeof(char)) );

	if(memPpal == NULL) {
		log_error(logUMV, "No hay suficiente espacio para reservar %d bytes", stack);
		exit(EXIT_FAILURE);
	}else{
		log_info(logUMV, "Memoria principal reservada exitosamente - %d bytes", stack);
	}

	log_debug(logUMV, "Creo un primer segmento vacio y libre");

	t_segmento * primerSegmento = (t_segmento*) malloc(sizeof(t_segmento));

	primerSegmento->cpuId      = -1;
	primerSegmento->pid        = -1;
	primerSegmento->base       = -1;
	primerSegmento->inicio     = 0;
	primerSegmento->libre      = true;
	primerSegmento->tamanio    = stack;

	//inicializo en vacio la memoria:
	integer i=0;
	for(i = 0; i < stack; i++){
		memPpal[i] = ' ';
		if(DEBUG){
			memPpal[i] = '.';
		}
	}
	memPpal[stack-1] = '\0';

	log_debug(logUMV, "Agrego el primer segmento a la lista de segmentos (inicial y vacia)");
	list_add(segmentos, primerSegmento);

	if(DEBUG){
		printf("memoria: %s\n\n",memPpal);
	}
	// Crea el hilo que escucha CPU y KERNEL
	pthread_t thKernelCpu;

	if (pthread_create(&thKernelCpu, NULL, listenCPUandKERNEL, (void *) puerto)){
		log_error(logUMV, "Hubo un problema al crear hilo de sockets - fuerzo la salida.");
		return EXIT_FAILURE;
	}

	//se queda escuchando comandos por consola para ejecutar
	while(running)
		consoleListener();

	// se termina la ejecucion, lebero recursos..
	log_destroy(logUMV);
	log_destroy(logUMVDump);
	list_destroy(segmentos);
	config_destroy(config);
	free(memPpal);
	pthread_rwlock_destroy(&lock);
	return EXIT_SUCCESS;
}


/*
 * Libera el dato pasado por argumento
 */
static void liberar(t_segmento *self){
	free(self);
}


/*
 * Devuelve los bytes de un segmento particular
 */
char* getBytes(integer base, integer offset, integer tamanio) {

	pthread_rwlock_rdlock(&lock);

//	log_info(logUMV, "tengo en la memoria:[INI]");
//	showMemory();
//	log_info(logUMV, "tengo en la memoria:[FIN]");

	integer j;
	integer inicio = _getDirectionByBase(base);
	integer size   = _getSizeByBase(base);

	char* buffer = string_new();

	if(inicio == BASE_INEXISTENTE){
		log_info(logUMV, "%sBASE_INEXISTENTE%s",green,none);
		string_append(&buffer, string_from_format(STR_BASE_INEXISTENTE));
	} else if((offset+tamanio) > size){
		log_info(logUMV, "%sSTACK_OVERFLOW!!!%s",green,none);
		string_append(&buffer, string_from_format(STR_STACK_OVERFLOW));
	}else{

		log_info(logUMV, "Todo ok, leo de la memoria..");

		integer inicio = _getDirectionByBase(base);

		integer j=0;
		for(j=0; j < tamanio; j++){
			string_append(&buffer, string_from_format("%c",memPpal[inicio+j+offset]));
		}
	}

	pthread_rwlock_unlock(&lock);

	return buffer;
}


/*
 * @synchronized
 * Recibo un buffer y en base a los parametros evaluo si esta ok o no
 */
integer writeBuffer(integer base, integer offset, integer tamanio, char* buffer){

	pthread_rwlock_wrlock(&lock);

//	log_info(logUMV, "Escribo en la base %d offset %d tamanio: %d el buffer: \n %s",
//			base, offset, tamanio, buffer);
	integer j;
	integer inicio  = _getDirectionByBase(base);
	integer size    = _getSizeByBase(base);
	integer retorno = EXIT_SUCCESS;

	if(inicio == BASE_INEXISTENTE){
		log_info(logUMV, "BASE_INEXISTENTE");
		retorno = BASE_INEXISTENTE;
	} else if((offset+tamanio) > size){
		log_info(logUMV, "STACK_OVERFLOW!!!");
		retorno = STACK_OVERFLOW;
	} else {
		log_info(logUMV, "Todo ok, escribo en memoria..");
		for(j=0; j < tamanio; j++){
			memPpal[inicio+j+offset] = buffer[j];
		}
	}
	pthread_rwlock_unlock(&lock);

	return retorno;
}


/*
 * Devuelve la direccion de inicio de una base dada
 */
integer _getDirectionByBase(integer base){

	integer j;
	for(j=0; j < list_size(segmentos); j++) {
		t_segmento * unSegmento = (t_segmento*) list_get(segmentos, j);
		if(base == unSegmento->base){
			return unSegmento->inicio;
		}
	}
	return BASE_INEXISTENTE;
}


/*
 * Devuelve el tamanio de un segmento de una base dada
 */
integer _getSizeByBase(integer base){

	integer j;
	for(j=0; j < list_size(segmentos); j++) {
		t_segmento * unSegmento = (t_segmento*) list_get(segmentos, j);
		if(base == unSegmento->base){
			return unSegmento->tamanio;
		}
	}
	return BASE_INEXISTENTE;
}


/*
 * Devuelve true si el cpu puede acceder a ese segmento
 * Esta funcionalidad tal vez no haga falta
 */
bool accessAllowed(integer cpuId, integer base){

	pthread_rwlock_rdlock(&lock);

	integer j;
	for(j=0; j < list_size(segmentos); j++) {
		t_segmento * unSegmento = (t_segmento*) list_get(segmentos, j);
		if(base == unSegmento -> base) {
			if(cpuId == unSegmento->cpuId) {
				return true;
			}else{
				return false;
			}
		}
	}

	pthread_rwlock_unlock(&lock);

}


void consoleListener() {
	usleep(550000);
	//lista de comandos
	char* dump     = "dump";
	char* compac   = "comp";
	char* crear    = "crea";
	char* eliminar = "elim";
	char* getbytes = "get";
	char* setbytes = "set";
	char* retardo  = "reta";
	char* algoFF   = "algo -ff";
	char* algoWF   = "algo -wf";
	char* mem      = "mem";
	char* play     = "play";
	char* test1    = "test1";
	char* test2    = "test2";
	char* cpus     = "cpus";
	char* sys      = "sys";
	char* end      = "end";
	char* off      = "off";
	char* on       = "on";

	char* comandos = "Comandos disponibles:\n"
			"\t'comp':.......................... Compacta la memoria \n"
			"\t'dump':.......................... Genera Dump\n"
			"\t'reta <tiempo>':................. Asigna un nuevo retardo\n"
			"\t'crea <pid> <tamanio>':.......... Crea un nuevo segmento \n"
			"\t'elim <pid>':.................... Elimina segmentos \n"
			"\t'get <base> <offset> <tamanio>':. Retorna bytes\n"
			"\t'set <base> <offset> <string>':.. Setea bytes en memoria\n"
			"\t'algo <algoritmo>':.............. Cambia el algoritmo (algo -wf|-ff)\n"
			"\t'mem':........................... Muestra la memoria actual\n"
			"\t'cpus':.......................... Cantidad de Cpus Activos\n"
			"\t'on|off':........................ Log en la consola activado/desactivado\n"
			"\t'sys <comando>':................. Comandos bash\n"
			"\t'end':........................... Finaliza la UMV\n";

	char comando[100];
	printf("%sIngrese un comando. %s%s%s >> ",red,cyan,comandos,none);
	scanf("%[^\n]%*c", comando);
	//system("clear");

	if (strcmp(comando,"") == 0){
		printError("[Comando vacio]");
		return;
	}else if (strcmp(comando,off) == 0){
		printComando("CONSOLA [OFF]:");
		logUMV->is_active_console = false;
	}else if (strcmp(comando,on) == 0){
		printComando("CONSOLA [ON]:");
		logUMV->is_active_console = true;
	}else if (strcmp(comando,end) == 0){
		printComando("[Fin]:");
		running = false;
	}else if (strcmp(comando,cpus) == 0){
		printComando("Cantidad de Cpus:");
		printf("Cantidad de Cpus activos: %d\n\n",cantCpus);
	}else if (strcmp(comando,dump) == 0){
		printComando("Dump");
		Dump();
	}else if (strcmp(comando,compac) == 0){
		printComando("Compactar");
		Compactar();
	}else if (strcmp(comando,test1) == 0){
		printComando("Test Compactar 1");
		testCompactar();
	}else if (strcmp(comando,test2) == 0){
		printComando("Test Compactar 2");
		testCompactar2();
	}else if (strcmp(comando,algoFF) == 0){
		printComando("Algoritmo First Fit");
		setAlgoritmo(ALGFF);
	}else if (strcmp(comando,algoWF) == 0){
		printComando("Algoritmo Worst Fit");
		setAlgoritmo(ALGWF);
	}else if (strcmp(comando,mem) == 0){
		printComando("Contenido de la memoria");
		showMemory();
	}else if (strcmp(comando,play) == 0){
		printComando("Play");
		Play();
	} else {

		char** split = get_string_as_array(comando);

		if (strcmp(split[0],eliminar) == 0){
			printComando("Eliminar");
			if(split[1]==NULL){
				printError("Faltan parametros");
			}else{
				integer pid = atoi(split[1]);
				eliminarSegmento(pid);
			}
		}else if (strcmp(split[0],sys) == 0){
			printComando("SysCall");
			if(split[1]==NULL){
				printError("Faltan parametros");
			}else{
				system(split[1]);
			}
		}else if (strcmp(split[0],retardo) == 0){
			printComando("Retardo");
			if(split[1]==NULL){
				printError("Faltan parametros");
			}else{
				integer value = atoi(split[1]);
				setRetardo(value);
			}
		}else if (strcmp(split[0],crear) == 0){
			printComando("Crear");
			if(split[1]==NULL || split[2]==NULL){
				printError("Faltan parametros");
			}else{
				integer pid = atoi(split[1]);
				integer tam = atoi(split[2]);
				crearSegmento(pid, tam);
			}
		}else if (strcmp(split[0],setbytes) == 0){
			printComando("Setear Bytes en un segmento");
			if(split[1]==NULL || split[2]==NULL || split[3]==NULL){
				printError("Faltan parametros");
			}else{
				integer base = atoi(split[1]);
				integer offs = atoi(split[2]);
				printf("Buffer a escribir en la memoria:'%s'\n\n",split[3]);
				writeBuffer(base, offs, strlen(split[3]), split[3]);
			}
		}else if (strcmp(split[0],getbytes) == 0){
			printComando("Obtener Bytes de un segmento");
			if(split[1]==NULL || split[2]==NULL || split[3]==NULL){
				printError("Faltan parametros");
			}else{
				integer base = atoi(split[1]);
				integer offs = atoi(split[2]);
				integer tama = atoi(split[3]);
				char* buffer = getBytes(base, offs, tama);
				printf("Buffer obtenido de memoria:'%s'\n\n",buffer);
			}
		}else{
			printError(comando);
		}
	}

}


/*
 * Setea el nuevo algoritmo de reservacion de segmentos
 */
void setAlgoritmo(integer alg){
	log_info(logUMV, "Se setea un nuevo algoritmo de creacion de segmentos:%d",alg);
	algoritmo = alg;
}


void printError(char* str1){
	printf("\n\t###### ERROR: Comando invalido: \"%s\" ######\n\n",str1);
}


void printComando(char* str1){
	printf("\n\t<<  %s  >>\n\n",str1);
}


/*
 * Devuelve una foto del estado de la memoria
 */
void Dump(){

	pthread_rwlock_rdlock(&lock);

	log_info(logUMVDump, "#######################################################");
	log_info(logUMVDump, "| Memoria:%7d  Segmentos:%4d  Compactaciones:%3d |", stack, list_size(segmentos),cantCompactaciones);
	log_info(logUMVDump, "-------------------------------------------------------");
	log_info(logUMVDump, "| Bloque | CpuId  |  PID   |  Base  | Inicio | Tamaño |");
	integer k;
	for(k = 0; k < list_size(segmentos); k++){
		t_segmento * unSegmento = (t_segmento*) list_get(segmentos, k);
		char bl = ' ';
		if(!(unSegmento->libre)){ bl = '#'; }

		log_info(logUMVDump, "|[%c]%4d | %6d | %6d | %6d | %6d | %6d |", bl, k, unSegmento->cpuId,
				unSegmento->pid, unSegmento->base, unSegmento->inicio, unSegmento->tamanio);

	}
	log_info(logUMVDump, "");
	pthread_rwlock_unlock(&lock);
}


/*
 * @synchronized
 */
void Compactar(){

	pthread_rwlock_wrlock(&lock);

	log_info(logUMV, "Compactar: Total de Segmentos: %d",list_size(segmentos));

	integer inicioSegmLibre = 0;
	integer tamanoSegmLibre = 0;

	integer i;
	for(i = 0 ;i < list_size(segmentos); i++){

		t_segmento * unSegmento = (t_segmento*) list_get(segmentos, i);
		if(unSegmento->libre) {
			//sumo el tamaño de memoria a remover
			tamanoSegmLibre += unSegmento->tamanio;

		} else { // es un bloque en uso

			inicioSegmLibre = _subirSegmento(unSegmento, inicioSegmLibre);
			unSegmento->inicio = inicioSegmLibre - unSegmento->tamanio;
			//inicioSegmLibre = unSegmento->inicio + unSegmento->tamanio;

			// actualizo el segmento en la lista
			list_replace(segmentos, i, unSegmento);

		}

	}

	log_info(logUMV, "Compactar: Se eliminan los segmentos libres...");
	integer cantSegmentos = list_size(segmentos);
	integer ii,jj;
	for(ii = 0 ;ii < cantSegmentos; ii++){ // me aseguro de borrar los segmentos vacios
		for(jj = 0 ;jj < list_size(segmentos); jj++){
			t_segmento * unSegmento = (t_segmento*) list_get(segmentos, jj);
			if(unSegmento->libre) {
				log_info(logUMV, "Compactar: Eliminando segmento:%d", jj);
				list_remove_and_destroy_element(segmentos, jj, (void*)liberar);
			}
		}
	}

	log_info(logUMV, "Compactar: Nuevo segmento libre al final de la lista de tamaño:%d", tamanoSegmLibre);
	t_segmento * nuevoSegmVacio = (t_segmento*) malloc(sizeof(t_segmento));

	nuevoSegmVacio->cpuId      = -1;
	nuevoSegmVacio->pid        = -1;
	nuevoSegmVacio->base       = -1;
	nuevoSegmVacio->inicio     = inicioSegmLibre;
	nuevoSegmVacio->libre      = true;
	nuevoSegmVacio->tamanio    = tamanoSegmLibre;

	list_add(segmentos, nuevoSegmVacio);

	cantCompactaciones++;
	log_info(logUMV, "COMPACTAR-Fin de compactacion.");

	pthread_rwlock_unlock(&lock);
}


/*
 * @private
 * mueve los segmentos de datos en la momoria real
 */
integer _subirSegmento(t_segmento* segmento, integer inicioSegmLibre){

	if(inicioSegmLibre == segmento->inicio){
		log_info(logUMV, "No se mueve el segmento porque es el primero y esta con datos");
		return (inicioSegmLibre + segmento->tamanio);
	}

	log_info(logUMV, "Subo segmento pid:%d desde %d hasta %d",
			segmento->pid, segmento->inicio, inicioSegmLibre);

	integer i = 0;
	integer sizeof_segm = segmento->tamanio;
	for(i = 0 ; i < sizeof_segm; i++) {
		memPpal[inicioSegmLibre+i] = memPpal[segmento->inicio+i];
	}

	return (inicioSegmLibre + segmento->tamanio);
}



void testCompactar2(){

		crearSegmento(3003, 1025);
		crearSegmento(3001, 10);
		crearSegmento(3001, 20);
		crearSegmento(3000, 30);
		crearSegmento(3001, 40);
		crearSegmento(4001, 500);
		crearSegmento(3000, 50);
		crearSegmento(3003, 1000);
		Dump();
		eliminarSegmento(3001);
		Dump();
		Compactar();
		Dump();
}


void testCompactarWF(){

		crearSegmento(3001, 102);
		crearSegmento(3001, 104);
		crearSegmento(3001, 220);
		crearSegmento(3001, 240);
		crearSegmento(500, 500);
		crearSegmento(50, 50);
		Dump();
		eliminarSegmento(3001);
		Dump();

}

/*
 * para probar la compactacion, probar una combinatoria de 5, con uno,
 * dos tres cuatro y cinco ocupados. en cada posicion,
 */
void testCompactar() {

	crearSegmento(3000, 10);
	crearSegmento(3001, 20);
	crearSegmento(3000, 30);
	crearSegmento(3001, 40);
	crearSegmento(3000, 50);
	Dump();
	eliminarSegmento(3001);
	Dump();
	Compactar();
	Dump();

}


/*Este hilo escucha las nuevas conexiones de CPU's y KERNEL*/
void* listenCPUandKERNEL(void* dato){

	log_info(logUMV, "Hilo de kernel y CPUs:");

	int sockfd, new_fd; // Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
	struct sockaddr_in my_addr; // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del cliente
	int sin_size;
	struct sigaction sa;
	int yes=1;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	my_addr.sin_family = AF_INET; // Ordenación de bytes de la maquina
	my_addr.sin_port = htons(puerto); // short, Ordenación de bytes de la red
	my_addr.sin_addr.s_addr = INADDR_ANY; // Rellenar con mi dirección IP
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	if (listen(sockfd, BACKLOGUMV) == -1) {
		perror("listen");
		exit(1);
	}
	sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	while(1) {
		sin_size = sizeof(struct sockaddr_in);
		log_info(logUMV, "ME QUEDO A LA ESPERA DE NUEVAS CONEXIONES");

		if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
			perror("accept");
			continue;
		}
		log_info(logUMV,"server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));

		// espero que se identifique el KERNEL o CPU (o sea, que haga el handshake)
		t_contenido mensaje;
		memset(mensaje, 0, sizeof(t_contenido));
		t_header header = recibirMensaje(new_fd, mensaje, logUMV);

		pthread_t thNew;

		if(header == CPU_TO_UMV_HANDSHAKE){
			log_info(logUMV, "Nueva conexion de un CPU a la UMV");
			pthread_create(&thNew, NULL, newCpu, (void *) new_fd);
		} else if(header == KRN_TO_UMV_HANDHAKE){
			log_info(logUMV, "Nueva conexion del KERNEL a la UMV");
			pthread_create(&thNew, NULL, newKernel, (void *) new_fd);
		} else {
			log_warning(logUMV, "CONEXION INESPERADA DE ALQUIEN QUE QUIERE MOLESTAR :/");
			close(new_fd);
		}
	}
	pthread_exit(0);
	return EXIT_FAILURE;
}


/*
 * esta funcion atiende la comunicacion con el CPU pasado por param
 */
void* newCpu(void* socketCpu){

	bool socketAlive = true;
	cantCpus++;

	log_info(logUMV, "Hilo de CPU: %d. Hay %d Cpus activos..",(int)socketCpu,cantCpus);

	while(socketAlive){

		t_contenido mensaje;
		memset(mensaje, 0, sizeof(t_contenido));
		t_header headerCpu = recibirMensaje(socketCpu, mensaje, logUMV);

		esperar();

		if(headerCpu == ERR_CONEXION_CERRADA){
			log_error(logUMV, "el socket que atiende este CPU (%d) se desconecto",socketCpu);
			socketAlive = false;
		} else if(headerCpu == CPU_TO_UMV_CAMBIO_PROCESO){
			log_info(logUMV, "CPU_TO_UMV_CAMBIO_PROCESO");
			integer pid = atoi(mensaje);
			cambioDeProceso(pid, headerCpu);
		} else if(headerCpu == CPU_TO_UMV_SOLICITAR_BYTES){

			log_info(logUMV,"CPU_TO_UMV_SOLICITAR_BYTES");
			char ** split = string_get_string_as_array(mensaje);
			integer base = atoi(split[0]);
			integer offset = atoi(split[1]);
			integer tamanio = atoi(split[2]);
			char* buffer = getBytes(base,offset,tamanio);
			if(strcmp(buffer,STR_STACK_OVERFLOW) == 0 || strcmp(buffer,STR_BASE_INEXISTENTE) == 0){
				enviarMensaje(socketCpu, UMV_TO_CPU_SEGM_FAULT, "", logUMV);
			}else{
				enviarMensaje(socketCpu, UMV_TO_CPU_BYTES_ENVIADOS, buffer, logUMV);
			}
			free(buffer);
		} else if(headerCpu == CPU_TO_UMV_SOLICITAR_ETIQUETAS){

			log_info(logUMV,"CPU_TO_UMV_SOLICITAR_ETIQUETAS");

			char ** split = string_get_string_as_array(mensaje);
			integer base = atoi(split[0]);
			integer offset = atoi(split[1]);
			integer tamanio = atoi(split[2]);

			char* buffer = malloc(sizeof(char)*tamanio);//= string_new();//[tamanio];// = getBytesTag(base,tamanio);

			pthread_rwlock_rdlock(&lock);

			integer inicio = _getDirectionByBase(base);
			memcpy(buffer, memPpal+inicio, tamanio);

			pthread_rwlock_unlock(&lock);

			send(socketCpu, buffer, tamanio, 0);

			//free(buffer);

		} else if(headerCpu == CPU_TO_UMV_ENVIAR_BYTES){
			log_debug(logUMV,"CPU_TO_UMV_ENVIAR_BYTES");

			char ** split = split_setBytes(mensaje);

			integer base = atoi(split[0]);
			integer offs = atoi(split[1]);
			string_trim(&split[2]);
			integer rta = writeBuffer(base, offs, strlen(split[2]), split[2]);

			if(rta==BASE_INEXISTENTE || rta==STACK_OVERFLOW){
				enviarMensaje(socketCpu, UMV_TO_CPU_SEGM_FAULT, "", logUMV);
			}else{
				enviarMensaje(socketCpu, UMV_TO_CPU_BYTES_RECIBIDOS, "", logUMV);
			}

		}else{
			log_warning(logUMV,"HEADER NO EVALUADO");
		}

	}

	cantCpus--;
	log_error(logUMV, "Se elimina hilo que atiende CPU (%d). Quedan %d Cpus activos\n",socketCpu,cantCpus);
	pthread_exit(0);
	return EXIT_SUCCESS;
}


/*
 * Se avisa cual es el proximo CPU que usar los segmentos de un PID determinado
 */
void cambioDeProceso(pid, cpuId){

	pthread_rwlock_wrlock(&lock);

	integer i;
	for(i = 0 ;i < list_size(segmentos); i++){
		t_segmento * unSegmento = (t_segmento*) list_get(segmentos, i);
		if(unSegmento->pid == pid) {
			unSegmento->cpuId = cpuId;
		}
	}

	pthread_rwlock_unlock(&lock);
}


/*
 * esta funcion atiende la comunicacion con el kernel
 */
void* newKernel(void* socketK){

	log_info(logUMV, "Se creo el hilo para atender al kernel, espero peticiones del kernel:");

	while(running){

		t_contenido mensaje;
		memset(mensaje, 0, sizeof(t_contenido));
		t_header headerK = recibirMensaje(socketK, mensaje, logUMV);

		esperar(); //espero aca asi no se pierde el mensaje enviado

		if(headerK == ERR_CONEXION_CERRADA){
			log_error(logUMV, "el Hilo que atiende el KERNEL se desconecto, "
					"por lo que no se puede continuar");
			running = false;
		} else if(headerK == KRN_TO_UMV_ELIMINAR_SEGMENTOS){
			integer pid = atoi(mensaje);
			log_warning(logUMV,"ELIMINAR SEGMENTO :%d",pid);
			eliminarSegmento(pid);
		} else if(headerK == KRN_TO_UMV_MEM_REQ){
			log_info(logUMV, "EL KERNEL SOLICITA CREAR SEGMENTO");

			char ** split = string_get_string_as_array(mensaje); // [234,4,6,7,45,8] -> split{0}=234 , split{1}=4

			integer pid = atoi(split[0]);
			integer tamanio = atoi(split[1]);
			log_warning(logUMV,"Crear SEGMENTO pid:%d,tamño:%d",pid,tamanio);
			integer base = crearSegmento(pid, tamanio);

			if(base == -1){
				log_info(logUMV, "No habia espacio para asignar pero reintento compactando una vez la mem");
				// intento una vez mas, compactando la memoria
				if(DEBUG){
					Dump();
				}
				Compactar();
				if(DEBUG){
					Dump();
				}
				base = crearSegmento(pid, tamanio);
			}

			if(base == -1){
				log_info(logUMV, "No habia espacio ni siquiera compactando");
				log_info(logUMV, "UMV_TO_KRN_MEMORY_OVERLOAD");
				enviarMensaje(socketK, UMV_TO_KRN_MEMORY_OVERLOAD, "", logUMV);
			}else{
				log_info(logUMV, "UMV_TO_KRN_SEGMENTO_CREADO - base %d",base);

				char* msj = string_from_format("%d", base);
				enviarMensaje(socketK, UMV_TO_KRN_SEGMENTO_CREADO, msj, logUMV);
			}
		} else if(headerK == KRN_TO_UMV_ENVIAR_ETIQUETAS){

			log_info(logUMV, "EL KERNEL envia segmento de etiquetas A LA UMV");

			int base = atoi(mensaje);

			enviarMensaje(socketK, KRN_TO_UMV_ENVIAR_ETIQUETAS, "", logUMV);

			pthread_rwlock_rdlock(&lock);
			int tamanio = _getSizeByBase(base);
			pthread_rwlock_unlock(&lock);

			char* etiquetas = malloc(sizeof(char)*tamanio);
			log_info(logUMV, "base:%d",base);

			int n = recv(socketK, etiquetas, tamanio, 0);

			writeBuffer(base, 0, n, etiquetas);

			enviarMensaje(socketK, UMV_TO_KRN_SEGMENTO_CREADO, "", logUMV);

			free(etiquetas);
		} else if(headerK == KRN_TO_UMV_ENVIAR_BYTES){
			log_info(logUMV, "EL KERNEL ENVIA BYTES A LA UMV");

			//char ** split = string_get_string_as_array(mensaje);
			char ** split = split_setBytes(mensaje);

			integer base = atoi(split[0]);
			integer offs = atoi(split[1]);
			writeBuffer(base, offs, strlen(split[2]), split[2]);
			enviarMensaje(socketK, KRN_TO_UMV_ENVIAR_BYTES, "BYTES RECIBIDOS OK", logUMV);

//		} else if(headerK == PRG_TO_KRN_CODE){
//			log_info(logUMV, "PRG_TO_KRN_CODE");
//			integer base = atoi(mensaje);
//			char* buffer = recibirCodigo(socketK, PRG_TO_KRN_CODE, logUMV);
//			writeBuffer(base,0,strlen(buffer),buffer);

		} else {
			log_error(logUMV, "LLEGO UN MENSAJE DEL KERNEL QUE NO SE ESPERABA!");
			esperar();
		}
	}
	
	log_error(logUMV, "NO DEBERIA SALIR NUNCA EL KERNEL (!) [FIN]");
	exit(EXIT_FAILURE);
	
}


char** split_setBytes(char* text) {
    int length_value = strlen(text) - 2;
    char* value_without_brackets = string_substring(text, 1, length_value);
    char **array_values = string_split(value_without_brackets, ",");

    int i = 3;
    while (array_values[i-1] != NULL && array_values[i] != NULL) {
    	string_append(&array_values[2], string_from_format(",%s",array_values[i]));
	    i++;
    }

    free(value_without_brackets);
    return array_values;
}


/*
 * Retorna la proxima base virtual para asignar segmentos,
 * y actualiza la base virtual;
 */
integer getRandomBase() {
	baseValue += (rand() % 4) + 1;
	return baseValue;
}

/*
 * @private
 * crear segmento
 */
integer crearSegmento(integer pid, integer tamanio){

	if(pid <= 0 || tamanio <= 0){
		log_error(logUMV, "los valores proceso y tamanio son invalidos: pid=%d - tamanio=%d",pid ,tamanio);
		return -1;
	}

	pthread_rwlock_wrlock(&lock);

	//veo que algoritmo de asignacion esta "de moda" y busco por ese algoritmo
	integer algAux = algoritmo; // lo guardo para usar aca por si lo cambian sobre la marcha

	if(algAux == ALGFF){ // busco el primer segmento vacio que entre el nuevo segmento
		log_info(logUMV, "Intento crear segmento mediante algoritmo FIRST-FIT");
	} else { // busco el segmento mas grande vacio que entre el nuevo segmento
		log_info(logUMV, "Intento crear segmento mediante algoritmo WORST-FIT");
	}

	log_debug(logUMV, "Crear Segmento: Se pide crear segmento de pid:%d y tamaño:%d",pid, tamanio);

	integer retorno = -1;

	// indice de la lista donde voy a meter el segmento nuevo
	integer indice = -1; // este valor es default para llevar a error

	// inicio del nuevo segmento,
	integer inicio = 0;

	// tamanio de la lista en este punto
	integer tamLista = list_size(segmentos);

	log_debug(logUMV,"Crear Segmento: Obtengo la lista de segmentos que tiene %d elementos", tamLista);

	integer mejorTamanio = 0;
	bool igualTamanio = false;
	bool encontrado = false; //variable que marca si encontro el segmento

	// recorro la lista de segmentos a ver cual es el primero vacio que entre el segmento
	integer i=0;
	for(i=0;i < tamLista; i++) {

		t_segmento* segmAux = list_get(segmentos, i);

		if(segmAux->libre == true) {

			log_debug(logUMV,"Crear Segmento: Encontre un segmento libre: i=%d",i);

			if(tamanio <= segmAux->tamanio) {
			
				log_debug(logUMV,"Crear Segmento: El elemento en %d de tamanio %d esta libre y tiene lugar", i, segmAux->tamanio);
				
				if(algAux == ALGFF){

					if(!encontrado) {

						encontrado = true;

						log_debug(logUMV, "Crear Segmento: Este segmento libre es el mejor lugar");
						indice = i;
						inicio = segmAux->inicio;

						if(tamanio == segmAux->tamanio){
							log_debug(logUMV,"Crear Segmento: La segmento vacio %d es de igual tamanio al entrante",i);
							igualTamanio = true;
						}else{
							igualTamanio = false;
						}
					}

				} else { // algAux === ALGWF

					if(segmAux->tamanio > mejorTamanio) {

						indice = i;
						mejorTamanio = segmAux->tamanio;
						inicio = segmAux->inicio;
						
						log_debug(logUMV, "Crear Segmento: Este segmento libre es el mejor lugar por ahora (WF)");
						
						if(tamanio == segmAux->tamanio){
							log_debug(logUMV,"Crear Segmento: La segmento vacio %d es de igual tamanio al entrante",i);
							igualTamanio = true;
						}else{
							igualTamanio = false;
						}

					}else{
						log_debug(logUMV, "Crear Segmento: el segmento estaba libre y entraba el segmento, pero ya habia uno mejor");
					}

				}

			} else { // no encontro un segmento vacio que entre el segmento nuevo
				
				log_debug(logUMV, "Crear Segmento: el segmento estaba libre pero era mas chico de lo solicitado");
			
			}
		}
	}

	if(indice != -1) {

		log_debug(logUMV, "Crear Segmento: Encontro un segmento donde entra");

		if(igualTamanio) { // si el tamanio es igual, sobreescribo la particion vacia y no genero nada nuevo
		
			log_debug(logUMV, "Crear Segmento: Los segmentos son de igual tamaño por lo que sobreescribe el segmento");
		
			t_segmento * segmentoNuevo = list_get(segmentos, indice);
			segmentoNuevo->libre  = false;
			segmentoNuevo->pid    = pid;
			segmentoNuevo->base   = getRandomBase();
			segmentoNuevo->inicio = inicio;
			segmentoNuevo->cpuId  = -1;

			list_replace(segmentos, indice, segmentoNuevo);

			// Había lugar y pudo guardar la partición
			retorno = segmentoNuevo->base;

		} else { // creo un nuevo segmento con el dato y modifico la vacia
			log_debug(logUMV, "Crear Segmento: La particion nueva es menor, genero una particion vacia nueva");

			/* la parte vacia cambia por la que tiene el dato
			 * creo la nueva particion para meter en la lista de particiones:*/
			t_segmento * segmentoNuevo = list_get(segmentos, indice);

			integer tamAux = segmentoNuevo->tamanio;

			segmentoNuevo->libre   = false;
			segmentoNuevo->pid     = pid;
			segmentoNuevo->cpuId   = -1;
			segmentoNuevo->inicio  = inicio;
			segmentoNuevo->base    = getRandomBase();
			segmentoNuevo->tamanio = tamanio;

			list_replace(segmentos, indice, segmentoNuevo);

			// creo la nueva segmento vacia que va despues de la particion con el dato, y de tamanio=resto
			t_segmento* segVacioRestante = (t_segmento*) malloc (sizeof(t_segmento));
			segVacioRestante->libre   = true;
			segVacioRestante->pid     = -1;
			segVacioRestante->cpuId   = -1;
			segVacioRestante->inicio  = inicio+tamanio;
			segVacioRestante->base    = -1;
			segVacioRestante->tamanio = tamAux -segmentoNuevo->tamanio;

			list_add_in_index(segmentos, indice+1, segVacioRestante);

			// Había lugar y pudo guardar la partición
			retorno = segmentoNuevo->base;
		}

	} else {

		log_debug(logUMV,"Crear Segmento: sin segmentos libres con espacio suficiente para almacenar %d bytes del pid:%d",tamanio,pid);
		retorno = -1; // -1 es que no habia espacio en memoria

	}

	pthread_rwlock_unlock(&lock);
	return retorno;
}


/*
 * Valida si en la cadena hay algun caracter que no es numero
 * devuelve true si *s es numerico
 */
bool strchk(char *s){
    bool r = false;

    while (*++s) {
        if (isdigit(s)) {
            r = true;
        } else {
            r = false;
            break;
        }
    }
    return r;
}


/*
 * Esta funcion libera la particion dentro del segmento de memoria
 * correspondiente al identificador enviado como parametro.
 */
void eliminarSegmento(integer pid){

	pthread_rwlock_wrlock(&lock);

	log_debug(logUMV, "Se eliminaran segmentos con PID=%d", pid);

	//integer listaSize = list_size(segmentos);
	log_debug(logUMV,"Cantidad de segmentos: %d", list_size(segmentos));

	log_info(logUMV,"Buscando segmentos a eliminar...");

	integer i=0;
	integer j=0;
	for(i=0; i < list_size(segmentos); i++){

		t_segmento * unSegmento = (t_segmento*) list_get(segmentos, i);

		if(unSegmento->pid == pid){

			j++;
			log_info(logUMV,"%d Encontrado!: posicion: %d - pid: %d", j, i, pid);

			unSegmento->libre = true;
			unSegmento->cpuId = -1;
			unSegmento->pid   = -1;

			list_replace(segmentos, i, unSegmento);
		}
	}

	if(j == 0){
		log_warning(logUMV, "Segmentos a eliminar no encontrados...");
	}

	pthread_rwlock_unlock(&lock);
}


/*
 * setea nuevo valor de retardo
 */
void setRetardo(integer nuevoTiempo){
	log_info(logUMV, "Se cambia el retardo de %d a %d milisegundos.", retardo, nuevoTiempo);
	retardo = nuevoTiempo;
}


/*
 * Se hace un sleep por el tiempo de milisegundos
 */
void esperar(){
	// valido que no sea cero, por si las moscas..
	if(retardo > 1){
		log_info(logUMV,"espero por %d milisegundos...",retardo);
		usleep(retardo*1000);
	}else{
		log_warning(logUMV,"/!\\ Valor de RETARDO invalido:%d",retardo);
	}
}

void sigchld_handler(int s){
	while(wait(NULL) > 0);
}


void showMemory(){
	//Cambio para poder mostrar los '\0' dentro de la memoria
	printf("\n");
	integer k;
	//for(k=0;k<strlen(memPpal);k++){
	for(k=0;k<stack;k++){
		printf("%s",string_from_format("%c",memPpal[k]));
	}
	printf("\n");
}

void Play(){
	system("sh play.sh");
}

/*
 * Igual a string_get_string_as_array solo que sin brackets y
 * con espacio para separar comandos
 */
char** get_string_as_array(char* text) {
    char **array_values = string_split(text, " ");
    int i = 0;
    while (array_values[i] != NULL) {
	    string_trim(&(array_values[i]));
	    i++;
    }
    return array_values;
}

