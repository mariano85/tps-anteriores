#include "kernel.h"


void interrupcion(t_process* aProcess, int32_t dir_memoria){
	t_contenido mensaje;
	t_header header = recibirMensaje(socketCPU, mensaje, logKernel);
	if(header == CPU_TO_KERNEL_IMPRIMIR_TEXTO){ // CAMBIAR POR CPU_TO_KERNEL_INTERRUPCION
			log_info(logKernel, "La CPU Me pide hacer una interrupcion :/");
			agregarProcesoColaBlock(aProcess ->tcb ->pid ,NO_SEMAPHORE);


		}


}

void bloquear(t_process* aProcess, int32_t id){

		t_contenido mensaje;
		t_header header = recibirMensaje(socketCPU, mensaje, logKernel);
		if(header == CPU_TO_KERNEL_IMPRIMIR_TEXTO){ // CAMBIAR POR CPU_TO_KERNEL_BLOQUEAR
				log_info(logKernel, "La CPU Me pide hacer un bloqueo :/");
				agregarProcesoColaBlock(aProcess ->tcb ->pid ,NO_SEMAPHORE);

			}

}

void crear_hilo(t_process* aProcess){

	t_contenido mensaje;
			t_header header = recibirMensaje(socketCPU, mensaje, logKernel);
			if(header == CPU_TO_KERNEL_IMPRIMIR_TEXTO){ // CAMBIAR POR CPU_TO_KERNEL_CREAR_HILO


				}

}

