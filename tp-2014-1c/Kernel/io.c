/*
 * io.c
 *
 *  Created on: 12/05/2014
 *      Author: utnso
 */
#include "kernel.h"
extern t_log* kernelLog;
static pthread_cond_t mutexes[];

t_list* GetProcessWaitingForMe(char* myName){

	/*Blocked Queue is locked right now, so, go go go*/
	bool _blocked_by_IO(void* element) {
		if (string_equals_ignore_case(((t_process*)element)->blockedByIO, myName)) {
			return true;
		}
		return false;
	}

	t_list* lista = list_filter(block_queue->elements, (void*)_blocked_by_IO);

	bool _any_blocked_of_mine(void* element) {

		bool _match_fd(void* element_to_compare) {
			if (((t_process*)element_to_compare)->process_fd == ((t_process*)element)->process_fd) {
				return true;
			}
			return false;
		}

		if (list_any_satisfy(lista, (void*)_match_fd)) {
			return true;
		}
		return false;
	}

	int counter = 0;
	while(counter < list_size(lista)){
		list_remove_by_condition(block_queue->elements, (void*)_any_blocked_of_mine);
		counter++;
	}
	return lista;
}

void* io(t_iothread* ioThread){

	int32_t myPID = process_get_thread_id();

	t_queue* myQueue = queue_create();

	log_info(kernelLog, "  IO Thread Started!(PID: %d)",myPID);
	log_info(kernelLog, string_from_format("Se agrego nuevo dispositivo: %s | Retardo: %i", ioThread->nombre, ioThread->retardo));

	for(;;){
		pthread_mutex_lock(&mutex_block_queue);	/* protect buffer */

		/* If there is nothing in the buffer then wait */
		while (block_queue->elements->elements_count == 0 ||
				(block_queue->elements->elements_count != 0
							&& !SomebodyHereByIO(block_queue->elements, ioThread->nombre)))
		{
			log_info(kernelLog, string_from_format("Thread IO (%s) says: i'm waiting for you, bitch!", ioThread->nombre));
			pthread_cond_wait(&mutexes[ioThread->mutexes_consumer], &mutex_block_queue);
		}

		log_info(kernelLog, string_from_format("(%s) IO Thread says: Miam...the critic section has a consumer inside! >:/", ioThread->nombre));

		//TODO Borrar comentarios inapropiados! :P
		t_list* new_elements = GetProcessWaitingForMe(ioThread->nombre);
		list_add_all(myQueue->elements, new_elements);
		pthread_cond_signal(&condpBlockedProcess);	/* wake up producer */
		pthread_mutex_unlock(&mutex_block_queue);	/* release the buffer */

		while(!list_is_empty(myQueue->elements)){

			t_process* aProcess = queue_pop(myQueue);
			log_info(kernelLog, string_from_format("(%s) IO Thread says: Process with PID: %d will use me for about %d execution units! :/", ioThread->nombre, aProcess->process_pcb->pId, aProcess->io_tiempo));

			usleep(ioThread->retardo * aProcess->io_tiempo);

			if(aProcess->process_fd != 0){
				addReadyProcess(aProcess);
				addExecProcess();
			}
			else{
				removeProcess(aProcess->process_pcb->pId, true);
			}
		}
	}

	return EXIT_SUCCESS;
}
