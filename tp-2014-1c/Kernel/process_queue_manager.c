/*
 * Sistemas Operativos - Esta Coverflow.
 * Grupo       : The codes remains the same.
 * Nombre      : process_queue_manager.c
 * Descripcion : Este archivo contiene la implementacion de las funciones para manejo de colas de estado implementadas.
 *
 */

#include "kernel.h"
t_log* kernelLog;
t_log* queueLog;
static pthread_cond_t mutexes[];

/**
 * @NAME: agregar nuevo proceso
 * @DESC: Agrega un proceso la lista compartida new_queue
 * usando semaforo mutex.
 */
void addNewProcess(t_process* aProcess) {
	int32_t SystemProcessCounter = 0;

	SystemProcessCounter = ObtenerCantidadDeProcesosEnSistema();
	if(config_kernel.MULTIPROG > SystemProcessCounter){
		addReadyProcess(aProcess);
	}
	else{
		pthread_mutex_lock (&mutex_new_queue);
		queue_push(new_queue, aProcess);
		pthread_mutex_unlock (&mutex_new_queue);
		log_info(kernelLog, "Se intentó encolar un nuevo proceso listo(Ready), pero el Sistema alcanzó el máximo de programas según grado de multiprogramación.");
		log_info(kernelLog, "Un nuevo proceso se insertó en la cola de listos!");
		PrintQueues();
	}
}

/*
 * Esta cola de procesos, deberia quedar ordenada por el peso del proceso en forma ascendente.
 * De esta manera se estaria poniendo en practica el algoritmo SJN de planificación.
 */
void addReadyProcess(t_process* aProcess) {
	t_process* process_aux;

	if(aProcess != NULL){
		process_aux = aProcess;
	}
	else{
		pthread_mutex_lock (&mutex_new_queue);
			/*Ordeno cola de NEW por algoritmo de SJN*/

			t_list* list_aux;
			list_aux =  new_queue->elements;
			list_sort(list_aux, (void*)ProcessLessWeightComparator);

			new_queue->elements = list_aux;

			/*Extraigo el primero mas liviano*/
			process_aux = queue_pop(new_queue);
		pthread_mutex_unlock (&mutex_new_queue);
	}

	/* Los Programas nuevos se rechazan solamente cuando falla un intento de reservarle un segmento de memoria.
	 * Si se alcanzara el grado de multiprogramación, los nuevos Programas quedarán a la espera, encolados en New.*/

	if(process_aux->in_umv == false){//Si no esta en memoria hacemos el tramite con la UMV
		log_info(kernelLog, "Voy a solicitar segmentos!");
		bool procesoAceptado = SolicitarSegmentosUMV(process_aux);

		/*Verifico haber podido reservar el espacio en memoria para mi proceso.*/
		if(procesoAceptado)
		{
			process_aux->in_umv = true;
			log_info(kernelLog, string_from_format("Se reservó con éxito el conjunto de segmentos para el programa (PID: %d)", process_aux->process_pcb->pId));
			log_info(kernelLog, "Comienza la operación de escritura en segmentos...");
			EscribirSegmentosUMV(process_aux);
			log_info(kernelLog, "Se terminó de escribir segmentos en la UMV existosamente");

			pthread_mutex_lock(&mutex_ready_queue);	/* Blocks the buffer */
			while ((ready_queue->elements->elements_count != 0) && list_any_satisfy(cpu_client_list, (void*)_cpuIsFree)){
//				log_info(kernelLog, "READY PUSH");
				pthread_cond_wait(&cond_ready_producer, &mutex_ready_queue);
			}
			/*Magic here, please!*/

					queue_push(ready_queue, process_aux);
					PrintQueues();
				pthread_cond_signal(&cond_ready_consumer);			/* wake up consumer */
			pthread_mutex_unlock(&mutex_ready_queue);	/* release the buffer */
		}
		else{

			log_error(kernelLog, string_from_format("No existe espacio suficiente para reservar el conjunto de segmentos para el programa. (PID: %d)", process_aux->process_pcb->pId));
			//ComunicarMuertePrograma(process_aux->process_fd);
			/*Devuelvo el proceso al mar (?)*/
			addExitProcess(process_aux);
		}
	}
	else{/*Ya estaba en memoria y pudo haber terminado de ejecutar recientemente*/

		pthread_mutex_lock(&mutex_ready_queue);	/* Blocks the buffer */
				queue_push(ready_queue, process_aux);
				PrintQueues();
		pthread_mutex_unlock(&mutex_ready_queue);	/* release the buffer */

	}

}

/*Cola NO sincronizada, porque la maneja un solo hilo! :)*/
void addExecProcess(){

	t_client_cpu* aCpu;
	t_process* aProcess;
	bool goGoGo = false;

	pthread_mutex_lock(&mutex_cpu_list);
//	log_debug(kernelLog, "BUSCO CPU LIBRE PARA EJECUTAR PROGRAMA!");
	if(list_any_satisfy(cpu_client_list, (void*)_cpuIsFree)){
		aCpu = list_find(cpu_client_list, (void*)_cpuIsFree);
		goGoGo = true;
	}
	pthread_mutex_unlock(&mutex_cpu_list);


	if(goGoGo){
		pthread_mutex_lock (&mutex_ready_queue);
//		log_debug(kernelLog, "SACO PROGRAMA DE READY!");
			aProcess = queue_pop(ready_queue);
		pthread_mutex_unlock (&mutex_ready_queue);

		if(aProcess != NULL){
//			log_debug(kernelLog, "HABIA UN PROGRAMA EN READY!");
			pthread_mutex_lock (&mutex_exec_queue);
				queue_push(exec_queue, aProcess);
			pthread_mutex_unlock (&mutex_exec_queue);


			aCpu->isBusy = true;
			aCpu->processFd = aProcess->process_fd;
			aCpu->processPID = aProcess->process_pcb->pId;
			log_info(kernelLog, string_from_format("Un nuevo programa entra en ejecución (PID: %d) en Procesador PID: %d", aProcess->process_pcb->pId, aCpu->cpuPID));

			enviarAEjecutar(aCpu->cpuFD, config_kernel.QUANTUM, aProcess);

			PrintQueues();
		}
		else{
			log_info(kernelLog, "Se intentó poner en ejecución un nuevo programa, pero no había ninguno en READY. :/");
			log_info(kernelLog, string_from_format("CPU (PID: %d) aguarda instrucciones!", aCpu->cpuPID));
		}
	}
	else{
		log_info(kernelLog, string_from_format("Se intentó poner en ejecución un nuevo programa, pero no había ningun CPU libre o activo! :/"));
	}
}

void addBlockedProcess(int32_t processFd, char* semaphoreKey, char* ioKey, int32_t io_tiempo){
	t_process* aProcess;
	bool _match_fd(void* element) {
		if (((t_process*)element)->process_fd == processFd) {
			return true;
		}
		return false;
	}

	/*Busco el proceso que se bloquea...obviamente tiene que estar en ejecucion!*/
	pthread_mutex_lock(&mutex_exec_queue);
		if((aProcess = (t_process*)list_find(exec_queue->elements, (void*)_match_fd)) != NULL){
			t_list* aux_exec_list = exec_queue->elements;
			list_remove_by_condition(aux_exec_list, _match_fd);
		}
	pthread_mutex_unlock(&mutex_exec_queue);

	/*Guardo el valor del semaforo en la estructura para saber por que se bloquea*/
	strcpy(aProcess->blockedBySemaphore, semaphoreKey);
	/*Guardo el valor del semaforo en la estructura para saber por que se bloquea*/
	strcpy(aProcess->blockedByIO, ioKey);
	aProcess->io_tiempo = io_tiempo;


	pthread_mutex_lock(&mutex_block_queue);/* protect buffer */
	if(!string_equals_ignore_case(ioKey, NO_IO)){
		t_iothread* iothread = GetIOThreadByName(ioKey);


		/* If there is something in the buffer, and anything in there that *has been blocked by IO (mean, no by semaphore)*, then wait */
		while ((block_queue->elements->elements_count != 0) && NoBodyHereBySemaphore(block_queue->elements)){
			pthread_cond_wait(&condpBlockedProcess, &mutex_block_queue);
		}

		queue_push(block_queue, aProcess);

		sleep(2);
		pthread_cond_signal(&mutexes[iothread->mutexes_consumer]);/* wake up consumer */
		pthread_mutex_unlock(&mutex_block_queue);/* release the buffer */
		log_info(kernelLog, string_from_format("El proceso PID %d se encuentra bloqueado por IO", aProcess->process_pcb->pId));
	}
	else
	{
		queue_push(block_queue, aProcess);
		log_info(kernelLog, string_from_format("El proceso PID %d se encuentra bloqueado por semaforo: %s", aProcess->process_pcb->pId, semaphoreKey));
		pthread_mutex_unlock(&mutex_block_queue);/* release the buffer */
	}

	PrintQueues();



}

void addExitProcess(t_process* aProcess){

	pthread_mutex_lock(&mutex_exit_queue);	/* protect buffer */
	while (exit_queue->elements->elements_count != 0){/* If there is something in the buffer then wait */
	  pthread_cond_wait(&cond_exit_producer, &mutex_exit_queue);
	}
	  //Makes things happen here! Inserts a new finished process into exit queue!
		log_info(kernelLog, string_from_format("Un nuevo proceso (PID: %i) ha finalizado. Se inserta en la cola de EXIT :)", aProcess->process_pcb->pId));
		queue_push(exit_queue, aProcess);
	  //-------------------------------------------------------------------------
		PrintQueues();
	pthread_cond_signal(&cond_exit_consumer);	/* wake up consumer */
	pthread_mutex_unlock(&mutex_exit_queue);	/* release the buffer */


}

/*Remove process by File Descriptor - Full Scann*/
void removeProcess(int32_t processPID, bool someoneKilledHim){

	bool _match_process_pid(void* element) {
		if (((t_process*)element)->process_pcb->pId == processPID) {
			return true;
		}
		return false;
	}

	pthread_mutex_lock (&mutex_block_queue);
	pthread_mutex_lock (&mutex_ready_queue);

	/*Ojo con DESBLOQUEAR ESTO! YA SE PIDIÓ ESTE RECURSO EN EL QUEUE_MANAGER AL INVOCAR ESTA FUNCION*/
	//pthread_mutex_lock (&mutex_exit_queue);
	pthread_mutex_lock (&mutex_exec_queue);
	pthread_mutex_lock (&mutex_new_queue);
	bool wasNew = false;



	/*Busco en qué cola está!*/
	t_process* aProcess;
	bool Hit = false; //Verifica si no se mato el proceso mientras lo buscaba

	if((aProcess = (t_process*)list_find(ready_queue->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(ready_queue->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(new_queue->elements, (void*)_match_process_pid)) != NULL){
		wasNew = true;
		Hit = true;
		list_remove_by_condition(new_queue->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(block_queue->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(block_queue->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(exec_queue->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(exec_queue->elements, (void*)_match_process_pid);
	}
	else if((aProcess = (t_process*)list_find(exit_queue->elements, (void*)_match_process_pid)) != NULL){
		Hit = true;
		list_remove_by_condition(exit_queue->elements, (void*)_match_process_pid);
	}
	pthread_mutex_unlock (&mutex_new_queue);
	pthread_mutex_unlock (&mutex_exec_queue);
	//pthread_mutex_unlock (&mutex_exit_queue);
	pthread_mutex_unlock (&mutex_ready_queue);
	pthread_mutex_unlock (&mutex_block_queue);

	if(Hit){
		if(!someoneKilledHim){/*Si nadie lo mató, entonces entró en la cola de EXIT y el PLP lo está expulsando del sistema*/
			ComunicarMuertePrograma(aProcess->process_fd, aProcess->in_umv);
		}
		/*Si el proceso que murió no era uno nuevo, entonces veo si por el gr. de multiprog puedo meter uno nuevo!*/
		if(!wasNew){
			CheckNewProcesses();
		}
		/*Si estaba en memoria borro los segmentos*/
		if(aProcess->in_umv){
			EliminarSegmentos(processPID);
		}

		KillProcess(aProcess);
	}
}

/*
 * @NAME: GetProcessStructureByAnSISOPCode
 * @DESC: Retorna una estructura de tipo t_process, con la informacion correspondiente.
 */
t_process* GetProcessStructureByAnSISOPCode(char* code, int32_t pid, int32_t fd){

	t_medatada_program* structData = metadata_desde_literal(code);
	t_process* proceso = malloc(sizeof(t_process));
	t_pcb* process_pcb = malloc(sizeof(t_pcb));
	proceso->scriptCode = string_new();
	proceso->process_pcb = process_pcb;
	proceso->process_pcb->indiceEtiquetas_size =structData->etiquetas_size;
	proceso->process_pcb->contextoActual_size = 0;
	proceso->process_pcb->pId = pid;
	proceso->process_pcb->programCounter = structData->instruccion_inicio;

	strcpy(proceso->blockedBySemaphore, NO_SEMAPHORE);
	strcpy(proceso->blockedByIO, NO_IO);
	proceso->io_tiempo = 0;
	proceso->scriptCode = string_duplicate(code);
	proceso->process_weight = GetProcessWeightByProperties(structData->cantidad_de_etiquetas, structData->cantidad_de_funciones, structData->instrucciones_size);
	proceso->process_fd = fd;
	proceso->in_umv = false;

	metadata_destruir(structData);

	return proceso;
}

/*
 * @NAME: GetProcessWeight
 * @DESC: Retorna el peso de un proceso, a partir de los datos del mismo, que recibe como parametros.
 */
int32_t GetProcessWeightByProperties(int32_t cantidadEtiquetas, int32_t cantidadFunciones, int32_t cantidadLineasCodigo){
	return (5 * cantidadEtiquetas + 3 * cantidadFunciones + cantidadLineasCodigo);
}

/*
 * @NAME: ProcessWeightComparator
 * @DESC: Compara el peso de dos procesos: elementA < elementB ?.
 */
bool ProcessLessWeightComparator(void* elementA, void* elementB){
	t_process* processA_aux = (t_process*)elementA;
	t_process* processB_aux = (t_process*)elementB;

	return processA_aux->process_weight < processB_aux->process_weight;
}

/*
 * @NAME: PrintReadyQueue
 * @DESC: Imprime el listado actual de elementos en la cola de READY
 *
 */
void PrintQueues(){

	log_info(queueLog, "NEW QUEUE -");
	PrintQueue(new_queue, mutex_new_queue, queueLog);
	log_info(queueLog, " ");
	log_info(queueLog, "READY QUEUE -");
	PrintQueue(ready_queue, mutex_ready_queue, queueLog);
	log_info(queueLog, " ");
	log_info(queueLog, "EXEC QUEUE -");
	PrintQueue(exec_queue, mutex_exec_queue, queueLog);
	log_info(queueLog, " ");
	log_info(queueLog, "BLOCK QUEUE -");
	PrintQueue(block_queue, mutex_block_queue, queueLog);
	log_info(queueLog, " ");
	log_info(queueLog, "EXIT QUEUE -");
	PrintQueue(exit_queue, mutex_exit_queue, queueLog);
	log_info(queueLog, " ");

}

/*
 *
 * Prints a queue, using mutex and a logger
 *
 */
void PrintQueue(t_queue* aQueue, pthread_mutex_t queueMutex, t_log* logger){

	t_list* list_aux = list_create();

//	pthread_mutex_lock (&queueMutex);
	list_add_all(list_aux, aQueue->elements);
//	pthread_mutex_unlock (&queueMutex);

	int counter;

	log_info(logger, "********************Process Status Report********************");
	if(list_size(list_aux) > 0){
		log_info(logger, "-----------------PID---------WEIGHT---------FD---------------");
		for(counter = 0; counter < list_size(list_aux); counter++){
			t_process* aProcess = (t_process*)list_get(list_aux, counter);
			log_info(logger, "-----------------%d-----------%d------------%d---------------", aProcess->process_pcb->pId, aProcess->process_weight, aProcess->process_fd);
		}
		log_info(logger, "*************************************************************");
	}
	else{
		log_info(logger, "There is any process here yet!\n");
	}
}


/*'Cause: El grado de multiprogramación indica la cantidad de Programas que
 *  pueden estar en los estados Ready, Blocked y Exec.
 *  */
int32_t ObtenerCantidadDeProcesosEnSistema(){

	int32_t counter = 0;

	pthread_mutex_lock (&mutex_block_queue);
	pthread_mutex_lock (&mutex_ready_queue);
	pthread_mutex_lock (&mutex_exec_queue);
		counter += ready_queue->elements->elements_count;
		counter += exec_queue->elements->elements_count;
		counter += block_queue->elements->elements_count;
	pthread_mutex_unlock (&mutex_exec_queue);
	pthread_mutex_unlock (&mutex_ready_queue);
	pthread_mutex_unlock (&mutex_block_queue);

	log_info(kernelLog, string_from_format("Cantidad de procesos en el sistema: %d", counter));

	return counter;
}

/* Hilo encargado de gestionar colas del kernel y vaciado de cola EXIT!
 * Utiliza semaforos de tipo productor - consumidor por grado de multiprogramacion
 */
void exit_queue_manager(void){

	int myPid = process_get_thread_id();
	log_info(kernelLog, "************** Exit Manager Thread Started (PID: %d) ***************",myPid);

	for(;;){

		pthread_mutex_lock(&mutex_exit_queue);	/* protect buffer */
		while (exit_queue->elements->elements_count == 0)/* If there is nothing in the buffer then wait */
		{
			pthread_cond_wait(&cond_exit_consumer, &mutex_exit_queue);
		}

		/*Make things happen here!*/
		log_debug(kernelLog, "Exit Queue Manager Thread Says: Hey!I Got you dude!");
		int32_t counter = 0;
		for(counter = 0; counter < exit_queue->elements->elements_count; counter ++ ){
			t_process* aProcess = (t_process*)queue_peek(exit_queue);

			removeProcess(aProcess->process_pcb->pId, false);
		}
		pthread_cond_signal(&cond_exit_producer);	/* wake up producer */
		pthread_mutex_unlock(&mutex_exit_queue);	/* release the buffer */
		PrintQueues();
		//-------------------------------------------------------------------------
	}
}

void ready_queue_manager(void){

	int myPid = process_get_thread_id();
	log_info(kernelLog, "**************Ready Queue Manager Thread Started (PID: %d) ***************",myPid);
	bool anyCpuFree = false;
	t_client_cpu* cpu;
	for(;;){
		anyCpuFree = false;
		//printf("pase por el queue manageeeeer");
		pthread_mutex_lock(&mutex_ready_queue);	/* protect buffer */
		while (ready_queue->elements->elements_count == 0 ||
					(!list_any_satisfy(cpu_client_list, (void*)_cpuIsFree)
						&& ready_queue->elements->elements_count != 0))/* If there is nothing in the buffer then wait */
		{
			if(!list_any_satisfy(cpu_client_list, (void*)_cpuIsFree)
					&& ready_queue->elements->elements_count != 0){
			log_info(kernelLog, "Queue Manager Thread Says: Un nuevo proceso listo! Pero ninguna CPU conectada o libre aún! :( No hago nada hasta que no levanten una!");
			}

			pthread_cond_wait(&cond_ready_consumer, &mutex_ready_queue);
		}
		/*Make things happen here!*/
//		pthread_mutex_lock(&mutex_cpu_list);
//			anyCpuFree = list_any_satisfy(cpu_client_list, (void*)_cpuIsFree);
//		pthread_mutex_unlock(&mutex_cpu_list);

//		if(anyCpuFree == true){
		pthread_mutex_unlock(&mutex_ready_queue);
			log_info(kernelLog, "Queue Manager Thread Says: Un nuevo proceso listo! Voy a pasarlo a ejecución!");
			addExecProcess();
//			pthread_mutex_unlock(&mutex_ready_queue);	/* release the buffer */
//			pthread_mutex_lock(&mutex_cpu_list);
//				cpu = list_find(cpu_client_list, (void*)_cpuIsFree);
//				cpu->isBusy = true;
//			pthread_mutex_unlock(&mutex_cpu_list);
//		}

		pthread_cond_signal(&cond_ready_producer);	/* wake up producer */
			/* release the buffer */

//		if(cpu != NULL && anyCpuFree){
//			addExecProcess(cpu);
//		}
		//-------------------------------------------------------------------------
	}
}

/*
 * @NAME: PrintReadyQueue
 * @DESC: Verifica si hay procesos nuevos en la cola de New para insertarlo en la cola de procesos listos.
 */
void CheckNewProcesses(){
	if(config_kernel.MULTIPROG > ObtenerCantidadDeProcesosEnSistema()){
		if(queue_size(new_queue) > 0){
			addReadyProcess(NULL);
		}
		else{
			log_info(kernelLog, "La cola de procesos nuevos (New-Queue) se encuentra vacía...");
		}
	}
}

/*
 * @NAME: NoBodyHereBySemaphore
 * @DESC: Recibe una lista de procesos y responde si nadie está por Semaforos
 */
bool NoBodyHereBySemaphore(t_list* aList){

	bool _not_blocked_by_semaphore(void* element) {
		//((t_process*)element)->blockedBySemaphore == NO_SEMAPHORE
			if (string_equals_ignore_case(((t_process*)element)->blockedBySemaphore, NO_SEMAPHORE)) {
				return true;
			}
			return false;
		}

	return list_all_satisfy(aList, (void*)_not_blocked_by_semaphore);
}

/*
 * @NAME: SomebodyHereByIO
 * @DESC: Recibe una lista de procesos y responde si alguno está bloqueado por IO
 */
bool SomebodyHereByIO(t_list* aList, char* ioKey){

	bool _blocked_by_IO(void* element) {
		//((t_process*)element)->blockedBySemaphore == NO_SEMAPHORE
			if (string_equals_ignore_case(((t_process*)element)->blockedByIO, ioKey)) {
				return true;
			}
			return false;
		}

	return list_any_satisfy(aList, (void*)_blocked_by_IO);

}

bool StillInside(int32_t processFd){

		bool _match_fd(void* element) {
			if (((t_process*)element)->process_fd == processFd) {
				return true;
			}
			return false;
		}

		t_process* aProcess;
		bool response = false;

		pthread_mutex_lock(&mutex_ready_queue);
		if((aProcess = (t_process*)list_find(ready_queue->elements, (void*)_match_fd)) != NULL){
			pthread_mutex_unlock(&mutex_ready_queue);
			response = true;
		}
		else{
			pthread_mutex_unlock(&mutex_ready_queue);
		}

		pthread_mutex_lock(&mutex_new_queue);
		if((aProcess = (t_process*)list_find(new_queue->elements, (void*)_match_fd)) != NULL){
			pthread_mutex_unlock(&mutex_new_queue);
			response = true;
		}
		else{
			pthread_mutex_unlock(&mutex_new_queue);
		}

		pthread_mutex_lock(&mutex_block_queue);
		if((aProcess = (t_process*)list_find(block_queue->elements, (void*)_match_fd)) != NULL){
			pthread_mutex_unlock(&mutex_block_queue);
			response = true;
		}
		else{
			pthread_mutex_unlock(&mutex_block_queue);
		}

		pthread_mutex_lock(&mutex_exec_queue);
		if((aProcess = (t_process*)list_find(exec_queue->elements, (void*)_match_fd)) != NULL){
			pthread_mutex_unlock(&mutex_exec_queue);
			response = true;
		}
		else{
			pthread_mutex_unlock(&mutex_exec_queue);
		}

		//pthread_mutex_lock(&mutex_exit_queue);
		if((aProcess = (t_process*)list_find(exit_queue->elements, (void*)_match_fd)) != NULL){
			//pthread_mutex_unlock(&mutex_exit_queue);
			response = true;
		}
		else{
			//pthread_mutex_unlock(&mutex_exit_queue);
		}

		return response;

}

int32_t GetProcessPidByFd(int32_t fd){

	bool _match_fd(void* element) {
		if (((t_process*)element)->process_fd == fd) {
			return true;
		}
		return false;
	}

	/*Busco en qué cola está!*/
	t_process* aProcess;

	pthread_mutex_lock(&mutex_ready_queue);
	if((aProcess = (t_process*)list_find(ready_queue->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_ready_queue);
		return aProcess->process_pcb->pId;
	}
	else{
		pthread_mutex_unlock(&mutex_ready_queue);
	}


	pthread_mutex_lock(&mutex_new_queue);
	if((aProcess = (t_process*)list_find(new_queue->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_new_queue);
		return aProcess->process_pcb->pId;
	}
	else{
		pthread_mutex_unlock(&mutex_new_queue);
	}

	pthread_mutex_lock(&mutex_block_queue);
	if((aProcess = (t_process*)list_find(block_queue->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_block_queue);
		return aProcess->process_pcb->pId;
	}
	else{
		pthread_mutex_unlock(&mutex_block_queue);
	}

	pthread_mutex_lock(&mutex_exec_queue);
	if((aProcess = (t_process*)list_find(exec_queue->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_exec_queue);
		return aProcess->process_pcb->pId;
	}
	else{
		pthread_mutex_unlock(&mutex_exec_queue);
	}

	pthread_mutex_lock(&mutex_exit_queue);
	if((aProcess = (t_process*)list_find(exit_queue->elements, (void*)_match_fd)) != NULL){
		pthread_mutex_unlock(&mutex_exit_queue);
		return aProcess->process_pcb->pId;
	}
	else{
		pthread_mutex_unlock(&mutex_exit_queue);
	}

	return 0;
}
