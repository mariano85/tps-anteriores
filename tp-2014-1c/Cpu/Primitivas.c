#include "Cpu.h"
#include "Primitivas.h"

t_log* logCpu;
int32_t quantum;
t_pcb* aPCB;
int32_t socketUMV;
int32_t socketKernel;
bool endFlag;
t_dictionary *variablesAnsisop;
bool contextSwitch;
bool bloqueado;
bool conError;


t_puntero definirVariable(t_nombre_variable identificador_variable){
	log_info(logCpu, "Comienzo a Definir Variable");
	t_contenido mensaje;
	int32_t direccion;
	char* string = string_new();
	string = string_duplicate(string_from_format("%c",identificador_variable));
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("[%d,%d,%c]",aPCB->segmentoStack,(aPCB->cursorStack - aPCB->segmentoStack),identificador_variable));
	enviarMensaje(socketUMV,CPU_TO_UMV_ENVIAR_BYTES,mensaje,logCpu);
	log_info(logCpu, string_from_format("Solicitud de escritura en segmento stack de una variable.....se envia: %c", identificador_variable));
	memset(mensaje, 0, sizeof(t_contenido));
	t_header header = recibirMensaje(socketUMV,mensaje,logCpu);
	log_info(logCpu, "Estoy en recibirMensaje");
	if(header == UMV_TO_CPU_BYTES_RECIBIDOS){
		log_info(logCpu, "La UMV me informa que la variable fue almacenada en el Stack. Variable definida correctamente!");
		log_info(logCpu, string_from_format("Voy a registrar la variable:%c", identificador_variable));
		direccion = (aPCB->cursorStack - aPCB->segmentoStack);
		dictionary_put(variablesAnsisop,string_substring(string,0,1),(void*)((aPCB->cursorStack - aPCB->segmentoStack)));
		log_info(logCpu, string_from_format("Var registrada:%d", (int32_t)dictionary_get(variablesAnsisop,string_from_format("%c",identificador_variable))));
		log_info(logCpu, string_from_format("Var registrada:%d", (int32_t)dictionary_get(variablesAnsisop,string_from_format("%c",identificador_variable))));
		aPCB->contextoActual_size++;
		log_info(logCpu, string_from_format("ContextoActualSize:%d", aPCB->contextoActual_size));
		//ver lo del diccionario
		aPCB->cursorStack = aPCB->cursorStack + 5;

	}
	else{

		log_info(logCpu, ":::ERROR::: STACK OVERFLOW!");
		quantum = 0;
		endFlag = false;
		bloqueado = false;
		conError = true;

	}


	return direccion;

}

	/*log_info(logCpu, "definirVariable:: Esta funcion define una variable registrandola donde corresponda");
	return (t_puntero)NULL;*/

t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable){
	//saco del diccionario el offset de la variable
	u_int32_t direccion_variable = -1;
	log_info(logCpu, "Busco en el diccionario la variable :%c",identificador_variable);
	log_info(logCpu, "Variables en diccionario: %d", dictionary_size(variablesAnsisop));
	log_info(logCpu, "Respuesta del diccionario:%d",dictionary_has_key(variablesAnsisop,"a"));
	log_info(logCpu, string_from_format("Var registrada:%d", (int32_t)dictionary_get(variablesAnsisop,string_from_format("%c",identificador_variable))));

	char* string = string_new();
	string = string_duplicate(string_from_format("%c",identificador_variable));
	log_info(logCpu,string_from_format("Estoy buscando la variable:%s",string));
	//log_info(logCpu,string_from_format("Registrados en Diccionario:%s y %s", variablesAnsisop->elements[0]->key,variablesAnsisop->elements[1]->key));
	if(dictionary_has_key(variablesAnsisop,string_substring(string,0,1))){

		direccion_variable = (u_int32_t)dictionary_get(variablesAnsisop,string_substring(string,0,1));


	}
	else{
		log_info(logCpu, ":::ERROR::: La variable requerida no se encuentra registrada en el sistema!");
		quantum = 0;
		endFlag = false;
		bloqueado = false;
		conError = true;


	}
	log_info(logCpu, string_from_format("Direccion variable:%d",direccion_variable));
	return direccion_variable;

/*log_info(logCpu, "obtenerPosicionVariable:: Esta funcion devuelve el offset de una variable dentro del stack");
	return (t_puntero)NULL;*/
}

t_valor_variable dereferenciar(t_puntero direccion_variable){
	/*if(direccion_variable == NULL){
		log_info(logCpu, ":::ERROR::: Estas tratando de desreferenciar una variable que no está");

	}*/
	t_contenido mensaje;
	t_valor_variable valor_variable;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje,string_from_format("[%d, %d, %d]",aPCB->segmentoStack,direccion_variable + 1,4));
	enviarMensaje(socketUMV,CPU_TO_UMV_SOLICITAR_BYTES, mensaje, logCpu);
	log_info(logCpu, string_from_format("Se le solicitan a la UMV los 4 bytes a partir del offset:%d", direccion_variable));
	memset(mensaje, 0, sizeof(t_contenido));
	t_header header = recibirMensaje(socketUMV, mensaje, logCpu);
	if(header == UMV_TO_CPU_BYTES_ENVIADOS){
		log_info(logCpu, "Recibo de la UMV el valor solicitado");
		valor_variable = atoi(mensaje);
	}
	else{
		log_info(logCpu, ":::ERROR:::SEGMENTATION FAULT!");
		quantum = 0;
		endFlag = false;
		bloqueado = false;
		conError = true;



	}
	return valor_variable;

}
	/*log_info(logCpu, "dereferenciar:: Esta funcion devuelve el valor de una variable dentro del stack");
	return (t_valor_variable)NULL;*/

void asignar(t_puntero direccion_variable, t_valor_variable valor){
	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("[%d,%d,%s]",aPCB->segmentoStack,(direccion_variable + 1),FillNumberWithZero(string_from_format("%d",valor),4)));
	enviarMensaje(socketUMV,CPU_TO_UMV_ENVIAR_BYTES, mensaje, logCpu);
	log_info(logCpu, string_from_format("Solicitud de escritura en segmento stack, asignando un valor a una variable.....se envia el valor: %d", valor));
	t_header header = recibirMensaje(socketUMV, mensaje, logCpu);
	if(header == UMV_TO_CPU_BYTES_RECIBIDOS){
			log_info(logCpu, "El valor ha sido asignado correctamente");

	}
	else{
			log_info(logCpu,":::ERROR:::  SEGMENTATION FAULT!");
			quantum = 0;
			endFlag = false;
			bloqueado = false;
			conError = true;



	}
}

/*log_info(logCpu, "asignar:: Esta funcion copia el valor en la direccion de una variable");*/

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	t_valor_variable valor_variable;
	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje,variable);
	enviarMensaje(socketKernel,SYSCALL_GET_REQUEST, mensaje, logCpu);
	log_info(logCpu, string_from_format("Solicitud a Kernel para obtener el valor de una variable compartida.....se envia el id de la variable: %s", mensaje));
	memset(mensaje, 0, sizeof(t_contenido));
	t_header header = recibirMensaje(socketKernel, mensaje, logCpu);
	if(header == KRN_TO_CPU_VAR_COMPARTIDA_OK){
			log_info(logCpu, "El valor ha sido recibido correctamente");
			valor_variable = atoi(mensaje);
			memset(mensaje, 0, sizeof(t_contenido));
			enviarMensaje(socketKernel,CPU_TO_KRN_OK, mensaje,logCpu);
	}
	else{
			log_info(logCpu,"Error al recibir mensaje de Kernel");
			memset(mensaje, 0, sizeof(t_contenido));
			enviarMensaje(socketKernel,CPU_TO_KRN_OK, mensaje,logCpu);
			quantum = 0;
			endFlag = false;
			conError = true;

	}
	return valor_variable;



/* "obtenerValorCompartida:: Solicita al Kernel obtener el valor de una variable compartida determinada");
	return (t_valor_variable)NULL;*/
}
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	t_contenido mensaje;
	t_valor_variable valor_retorno;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("[%s, %d]",variable,valor));
	enviarMensaje(socketKernel,SYSCALL_SET_REQUEST, mensaje,logCpu);
	log_info(logCpu, string_from_format("Solicitud a Kernel para asignar un valor a una variable compartida.....se envia el id de la variable:%s y el valor a asignar:%d",variable,valor));
	memset(mensaje, 0, sizeof(t_contenido));
	t_header header = recibirMensaje(socketKernel, mensaje,logCpu);
	if(header == KRN_TO_CPU_ASIGNAR_OK){
		log_info(logCpu, "El valor ha sido almacenado correctamente");
		valor_retorno = atoi(mensaje);
		memset(mensaje, 0, sizeof(t_contenido));
		enviarMensaje(socketKernel,CPU_TO_KRN_OK, mensaje,logCpu);
	}
	else{
		log_info(logCpu, "El valor no ha sido almacenado");
		quantum = 0;
		endFlag = false;
		conError = true;


	}
	return valor_retorno;
//mensaje tiene el valor que guardó el kernel

	/*log_info(logCpu, "asignarValorCompartida:: Solicita al Kernel asignarle un valor a una variable compartida determinada");
	return (t_valor_variable)NULL;*/
}

void irAlLabel(t_nombre_etiqueta etiqueta){
	log_info(logCpu, string_from_format("Entro en irAlLabel con etiqueta:%s\n\n",etiqueta));
	t_contenido mensaje;
	t_size etiquetas_size = aPCB->indiceEtiquetas_size;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("[%d, %d, %d]", aPCB->indiceEtiquetas,0,etiquetas_size));
	enviarMensaje(socketUMV,CPU_TO_UMV_SOLICITAR_ETIQUETAS, mensaje, logCpu);
	log_info(logCpu, "Se envía mensaje a UMV para obtener las etiquetas serializadas");
	memset(mensaje, 0, sizeof(t_contenido));
	char* etiquetas = (char*)malloc(sizeof(char)*etiquetas_size);
	log_info(logCpu, "Recibo el serializado de las etiquetas");
	int n = recv(socketUMV,etiquetas,512,0);
	log_info(logCpu, string_from_format("Recibo :%i bytes", n));
	log_info(logCpu, string_from_format("Serializado:%s",etiquetas));
	t_puntero_instruccion puntero_instruccion = metadata_buscar_etiqueta(etiqueta, etiquetas, etiquetas_size);
	free(etiquetas);
	if(puntero_instruccion >= 0){
				log_info(logCpu, string_from_format("Me estoy yendo a la etiqueta:%s", etiqueta));
				log_info(logCpu, string_from_format("Me estoy yendo al PC:%d",puntero_instruccion));
				aPCB->programCounter = (puntero_instruccion - 1);

	}
	else{

		log_info(logCpu,":::ERROR::: SEGMENTATION FAULT!");
		quantum = 0;
		endFlag = false;
		conError = true;

	}



	//t_header header = recibirMensaje(socketUMV, mensaje,logCpu);
	//if(header == UMV_TO_CPU_BYTES_ENVIADOS){
		//log_info(logCpu,"Etiquetas recibidas desde UMV");
		//char* etiquetas = string_new();







//
//				int x;
//				for(x=0; x <etiquetas_size; x++){
//					char b = '\0';
//					if(mensaje[x] == '_'){
//						string_append(&etiquetas, string_from_format("%c", b));
//						log_info(logCpu, string_from_format("%c", mensaje[x]));
//					}
//					else{
//						string_append(&etiquetas, string_from_format("%c", mensaje[x]));
//						log_info(logCpu, string_from_format("%c", mensaje[x]));
//					}
//					//string_append(&etiquetas, string_from_format("%c", ((mensaje[x] == '_') ? '\0' : string_from_format("%c",mensaje[x])));
//				}

		//int j = 0;
		//while(j < etiquetas_size){
			//string_append(&etiquetas, string_from_format("%c", mensaje[j]));
			//j++;
		}

		//log_info(logCpu, string_from_format("Diccionario Serializado de Etiquetas:%s", mensaje));
		//t_puntero_instruccion puntero_instruccion = metadata_buscar_etiqueta(etiqueta, etiquetas, etiquetas_size);



	/*log_info(logCpu, "irAlLabel:: Devuelve el numero de la primer instruccion ejecutable de etiqueta");*/



void llamarSinRetorno(t_nombre_etiqueta etiqueta){
	log_info(logCpu, "Llamada a llamarSinRetorno");
	t_contenido mensaje;

	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("[%d,%d,%s]",aPCB->segmentoStack,(aPCB->cursorStack - aPCB->segmentoStack),FillNumberWithZero(string_from_format("%d",aPCB->contextoActual_size),4)));
	enviarMensaje(socketUMV,CPU_TO_UMV_ENVIAR_BYTES, mensaje, logCpu);
	log_info(logCpu,"Se envía mensaje a UMV para almacenar el tamaño contexto actual");
	memset(mensaje, 0, sizeof(t_contenido));
	t_header header = recibirMensaje(socketUMV, mensaje,logCpu);
	if(header == UMV_TO_CPU_BYTES_RECIBIDOS){
			log_info(logCpu,"Se guardó correctamente el tamaño del contexto actual en la UMV");
			aPCB->cursorStack = aPCB->cursorStack + 4;
			memset(mensaje, 0, sizeof(t_contenido));
			strcpy(mensaje, string_from_format("[%d,%d,%s]",aPCB->segmentoStack,(aPCB->cursorStack - aPCB->segmentoStack),FillNumberWithZero(string_from_format("%d",aPCB->programCounter + 1),4)));
			enviarMensaje(socketUMV,CPU_TO_UMV_ENVIAR_BYTES, mensaje,logCpu);
			log_info(logCpu, "Se envía mensaje a UMV para almacenar el program counter");
			memset(mensaje, 0, sizeof(t_contenido));
			header = recibirMensaje(socketUMV, mensaje,logCpu);
			if(header == UMV_TO_CPU_BYTES_RECIBIDOS){
				log_info(logCpu,"Se guardó correctamente el program counter en la UMV");
				dictionary_clean(variablesAnsisop);
				aPCB->contextoActual_size = 0;
				aPCB->cursorStack = aPCB->cursorStack + 4;
				log_info(logCpu, string_from_format("Llamo a irAlLAbel con etiqueta:%s", etiqueta));
				irAlLabel(etiqueta);
			}
			else{
				log_info(logCpu,":::ERROR::: STACK OVERFLOW!");
				quantum = 0;
				endFlag = false;
				conError = true;

				aPCB->cursorStack = aPCB->cursorStack - 4;
			}
		}
		else{
			log_info(logCpu,":::ERROR::: STACK OVERFLOW!");
			quantum = 0;
			conError = true;
			endFlag = false;
		}



	/*log_info(logCpu, "llamarSinRetorno:: Preserva el contexto actual y demas");
	return (t_puntero_instruccion)NULL;*/
}


void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	log_info(logCpu, "Solicito llamarConRetorno");
	t_contenido mensaje;
	//int32_t contAct = aPCB->contextoActual_size;
	//int32_t programCounter = aPCB->programCounter + 1;

	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("[%d,%d,%s]",aPCB->segmentoStack,(aPCB->cursorStack - aPCB->segmentoStack),FillNumberWithZero(string_from_format("%d",aPCB->contextoActual_size),4)));
	enviarMensaje(socketUMV,CPU_TO_UMV_ENVIAR_BYTES, mensaje, logCpu);
	log_info(logCpu, "Se envía mensaje a UMV para almacenar el tamaño contexto actual");
	memset(mensaje, 0, sizeof(t_contenido));
	t_header header = recibirMensaje(socketUMV, mensaje,logCpu);
	if(header == UMV_TO_CPU_BYTES_RECIBIDOS){
		log_info(logCpu,"Se guardó correctamente el tamaño del contexto actual en la UMV");
		aPCB->cursorStack = aPCB->cursorStack + 4;
		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d,%d,%s]",aPCB->segmentoStack,(aPCB->cursorStack - aPCB->segmentoStack),FillNumberWithZero(string_from_format("%d",aPCB->programCounter + 1),4)));
		enviarMensaje(socketUMV,CPU_TO_UMV_ENVIAR_BYTES, mensaje,logCpu);
		log_info(logCpu, "Se envía mensaje a UMV para almacenar el program counter");
		memset(mensaje, 0, sizeof(t_contenido));
		header = recibirMensaje(socketUMV, mensaje,logCpu);
		if(header == UMV_TO_CPU_BYTES_RECIBIDOS){
			log_info(logCpu,"Se guardó correctamente el program counter en la UMV");
			aPCB->cursorStack = aPCB->cursorStack + 4;
			memset(mensaje, 0, sizeof(t_contenido));
			strcpy(mensaje, string_from_format("[%d,%d,%s]",aPCB->segmentoStack,(aPCB->cursorStack - aPCB->segmentoStack),FillNumberWithZero(string_from_format("%d",donde_retornar),4)));
			enviarMensaje(socketUMV,CPU_TO_UMV_ENVIAR_BYTES, mensaje,logCpu);
			log_info(logCpu, "Se envía mensaje a UMV para almacenar el program counter");
			memset(mensaje, 0, sizeof(t_contenido));
			header = recibirMensaje(socketUMV, mensaje,logCpu);
			if(header == UMV_TO_CPU_BYTES_RECIBIDOS){
				log_info(logCpu,"Se guardó correctamente la direccion donde retornar en la UMV");
				dictionary_clean(variablesAnsisop);
				aPCB->contextoActual_size = 0;
				aPCB->cursorStack = aPCB->cursorStack + 4;
				irAlLabel(etiqueta);
				contextSwitch = true;
			}
			else{
				log_info(logCpu,":::ERROR::: STACK OVERFLOW!");
				quantum = 0;
				endFlag = false;
				conError = true;
				aPCB->cursorStack = aPCB->cursorStack - 8;

			}
		    }
		else{
			log_info(logCpu,":::ERROR::: STACK OVERFLOW!");
			quantum = 0;
			endFlag = false;
			conError = true;
			aPCB->cursorStack = aPCB->cursorStack - 4;
		}
		}
	else{
		log_info(logCpu,":::ERROR::: STACK OVERFLOW!");
		quantum = 0;
		endFlag = false;
		conError = true;

	}



/*log_info(logCpu,"llamarConRetorno:: Preserva el contexto actual y demas");
	return (t_puntero_instruccion)NULL;*/
}


void finalizar(){

	t_contenido mensaje;
	int32_t tamCA;
	char id[1];
	int32_t dirVar;


	//desapilo con el diccionario
	int32_t aDesapilar = dictionary_size(variablesAnsisop);
	log_info(logCpu, string_from_format("Cantidad de Variables antes de finalizar el contexto:%d", aDesapilar));

	while(aDesapilar > 0){
		aPCB->cursorStack = aPCB->cursorStack - 5;
		aDesapilar--;
	}
	log_info(logCpu, string_from_format("Cursor Stack:%d", aPCB->cursorStack));
	if(aPCB->cursorStack == aPCB->segmentoStack){
		log_info(logCpu, "Esta finalizando el contexto principal!");
		log_info(logCpu, "Variables en diccionario: %d", dictionary_size(variablesAnsisop));
		quantum = 0;
		endFlag = true;
		int32_t cantVar = dictionary_size(variablesAnsisop);

			memset(mensaje, 0, sizeof(t_contenido));
			strcpy(mensaje, string_from_format("[%d,%d,%d]",aPCB->segmentoStack,0,cantVar * 5));
			enviarMensaje(socketUMV,CPU_TO_UMV_SOLICITAR_BYTES,mensaje,logCpu);
			log_info(logCpu, "Envio mensaje para recuperar los ultimos valores de las variables del Contexto Principal");
			memset(mensaje, 0, sizeof(t_contenido));
			recibirMensaje(socketUMV,mensaje,logCpu);
			char* string = string_new();
			string = string_duplicate(mensaje);
			strcpy(mensaje, string_from_format("[%d,%s]", aPCB->pId, string));
			enviarMensaje(socketKernel, CPU_TO_KRN_END_PROC, mensaje, logCpu);
			log_info(logCpu, "Se envía mensaje a Kernel por finalización de un programa!");
			endFlag = true;

	}
	else{

		aPCB->cursorStack = aPCB->cursorStack - 4;

		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d,%d,%d]",aPCB->segmentoStack,(aPCB->cursorStack - aPCB->segmentoStack),4));
		enviarMensaje(socketUMV,CPU_TO_UMV_SOLICITAR_BYTES, mensaje,logCpu);
		log_info(logCpu, "Se envía mensaje a UMV para recuperar el contexto anterior");
		memset(mensaje, 0, sizeof(t_contenido));
		recibirMensaje(socketUMV, mensaje, logCpu);

		aPCB->programCounter = atoi(mensaje);
		contextSwitch = true;
		aPCB->cursorStack = aPCB->cursorStack - 4;

		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d,%d,%d]",aPCB->segmentoStack,(aPCB->cursorStack - aPCB->segmentoStack),4));
		enviarMensaje(socketUMV,CPU_TO_UMV_SOLICITAR_BYTES, mensaje,logCpu);
		log_info(logCpu, "Se envió mensaje a UMV para recuperar el contexto anterior");
		memset(mensaje, 0, sizeof(t_contenido));
		recibirMensaje(socketUMV, mensaje, logCpu);

		aPCB->contextoActual_size = atoi(mensaje);

		tamCA = aPCB->contextoActual_size;
		int32_t offset = (aPCB->cursorStack - aPCB->segmentoStack) - (tamCA * 5);


		int32_t i;

		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d,%d,%d]",aPCB->segmentoStack,offset,tamCA * 5));
		enviarMensaje(socketUMV,CPU_TO_UMV_SOLICITAR_BYTES, mensaje,logCpu);
		log_info(logCpu, "Se envía mensaje a UMV para recuperar el diccionario del contexto actual");

		memset(mensaje, 0, sizeof(t_contenido));
		recibirMensaje(socketUMV, mensaje, logCpu);

		dictionary_clean(variablesAnsisop);


		char* string = string_new();
		string = string_duplicate(mensaje);

		int32_t start = 1;
		int32_t x = 0;
		for(i = 0;i<tamCA;i++){

			id[0] = string[x];
			dirVar = offset + (i*5);
			dictionary_put(variablesAnsisop, string_from_format("%c",id[0]), (void*)(dirVar));
			x = x + 5;
			start = start + 5;
		}
 	}




/*	log_info(logCpu, "finalizar:: Cambia el contexto actual por el anterior");
	return (t_puntero_instruccion)NULL;*/
}

void retornar(t_valor_variable retorno){
	t_contenido mensaje;
	int32_t donde_retornar;
	int32_t tamCA;
	char id[1];
	int32_t dirVar;


	//desapilo con el diccionario
	int32_t aDesapilar = dictionary_size(variablesAnsisop);
	while(aDesapilar > 0){
		aPCB->cursorStack = aPCB->cursorStack - 5;
		aDesapilar--;
	}

		aPCB->cursorStack = aPCB->cursorStack - 4;

		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d,%d,%d]",aPCB->segmentoStack,(aPCB->cursorStack - aPCB->segmentoStack),4));
		enviarMensaje(socketUMV,CPU_TO_UMV_SOLICITAR_BYTES, mensaje,logCpu);
		log_info(logCpu, "Se envía mensaje a UMV para recuperar el contexto anterior");

		memset(mensaje, 0, sizeof(t_contenido));
		recibirMensaje(socketUMV, mensaje,logCpu);
		donde_retornar = atoi(mensaje);

		aPCB->cursorStack = aPCB->cursorStack - 4;

		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d,%d,%d]",aPCB->segmentoStack,(aPCB->cursorStack - aPCB->segmentoStack),4));
		enviarMensaje(socketUMV,CPU_TO_UMV_SOLICITAR_BYTES, mensaje,logCpu);
		log_info(logCpu, "Se envía mensaje a UMV para recuperar el contexto anterior");

		memset(mensaje, 0, sizeof(t_contenido));
		recibirMensaje(socketUMV, mensaje,logCpu);
		aPCB->programCounter = atoi(mensaje);
		contextSwitch = true;
		aPCB->cursorStack = aPCB->cursorStack - 4;

		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d,%d,%d]",aPCB->segmentoStack,(aPCB->cursorStack - aPCB->segmentoStack),4));
		enviarMensaje(socketUMV,CPU_TO_UMV_SOLICITAR_BYTES, mensaje,logCpu);
		log_info(logCpu, "Se envía mensaje a UMV para recuperar el contexto anterior");

		memset(mensaje, 0, sizeof(t_contenido));
		recibirMensaje(socketUMV, mensaje,logCpu);
		aPCB->contextoActual_size = atoi(mensaje);

		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d,%d,%s]",aPCB->segmentoStack,donde_retornar + 1,FillNumberWithZero(string_from_format("%d",retorno),4)));
		enviarMensaje(socketUMV,CPU_TO_UMV_ENVIAR_BYTES, mensaje,logCpu);
		log_info(logCpu, string_from_format("Se asigna el valor en la direccion de retorno:%d", donde_retornar));
		log_info(logCpu, "Se envía mensaje a UMV para asignar el valor de retorno");
		recibirMensaje(socketUMV, mensaje,logCpu);

		tamCA = aPCB->contextoActual_size;
		int32_t offset = (aPCB->cursorStack - aPCB->segmentoStack) - (tamCA * 5);

		int32_t i;


		memset(mensaje, 0, sizeof(t_contenido));
		strcpy(mensaje, string_from_format("[%d,%d,%d]",aPCB->segmentoStack,offset,tamCA * 5));
		enviarMensaje(socketUMV,CPU_TO_UMV_SOLICITAR_BYTES, mensaje,logCpu);
		log_info(logCpu, "Se envía mensaje a UMV para recuperar el diccionario del contexto actual");
		memset(mensaje, 0, sizeof(t_contenido));
		recibirMensaje(socketUMV, mensaje,logCpu);

		dictionary_clean(variablesAnsisop);
		char* string = string_new();
		string = string_duplicate(mensaje);
		int32_t start = 1;
		int32_t x = 0;
		for(i = 0;i<tamCA;i++){

			id[0] = string[x];
			dirVar = offset + (i*5);
			dictionary_put(variablesAnsisop, string_from_format("%c",id[0]), (void*)(dirVar));
			x = x + 5;
			start = start + 5;
		}
 	

	/*log_info(logCpu, "retornar:: retornar");
	return (t_puntero_instruccion)NULL;*/
}

void imprimir(t_valor_variable valor){
	if(!conError){
		log_info(logCpu, string_from_format("Llega el valor:%d para imprimirse", valor));
	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje,string_from_format("%d",valor));

	enviarMensaje(socketKernel,CPU_TO_KRN_IMPRIMIR, mensaje,logCpu);
	log_info(logCpu,string_from_format("Se envia el valor:%d para que kernel lo envíe al Programa y éste lo imprima por pantalla", valor));
	}
	else{
		log_info(logCpu, "No imprimo nada por un error previo!");
	}

	/*log_info(logCpu, "imprimir:: Envia al kernel el valor de una variable para que se lo envie al programa en ejecucion");
	return (int32_t)0;*/
}

void imprimirTexto(char* texto){
	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, texto);
	enviarMensaje(socketKernel,CPU_TO_KRN_IMPRIMIR_TEXTO, mensaje,logCpu);
	log_info(logCpu,string_from_format("Se envia la cadena: %s para que kernel lo envíe al Programa y éste lo imprima por pantalla", mensaje));

	/*log_info(logCpu, "imprimirTexto:: Envia al kernel una cadena de texto para que se la envie al programa en ejecucion");
	return (int32_t)0;*/
}

void entradaSalida(t_nombre_dispositivo dispositivo, int32_t tiempo){
	t_contenido mensaje;
	int32_t pid            = aPCB->pId;
	int32_t contAct        = aPCB->contextoActual_size;
	int32_t programCounter = aPCB->programCounter;
	int32_t cursorStack    = aPCB->cursorStack;

	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje,string_from_format("[%d,%s,%d,%d,%d,%d]", pid, dispositivo, tiempo, programCounter, contAct, cursorStack));
	enviarMensaje(socketKernel,SYSCALL_IO_REQUEST, mensaje,logCpu);
	log_info(logCpu, string_from_format("Se envia solicitud de IO al Kernel para el dispositivo: %s, de un tiempo de:%d",dispositivo,tiempo));

	quantum = 0;
	endFlag = false;
	bloqueado = true;
	

	/*log_info(logCpu, "entradaSalida:: Informa al kernel que el programa actual pretende usar un dispositivo");*/
}

void wait(t_nombre_semaforo identificador_semaforo){
	t_contenido mensaje;
	int32_t pid            = aPCB->pId;
	int32_t contAct        = aPCB->contextoActual_size;
	int32_t programCounter = aPCB->programCounter;
	int32_t cursorStack    = aPCB->cursorStack;

	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje, string_from_format("[%s,%d,%d,%d,%d]",identificador_semaforo,pid,programCounter,contAct,cursorStack));
	enviarMensaje(socketKernel,SYSCALL_WAIT_REQUEST, mensaje,logCpu);
	log_info(logCpu, string_from_format("Se envia solicitud de WAIT al Kernel para el semaforo:%s",identificador_semaforo));
	memset(mensaje, 0, sizeof(t_contenido));
	t_header header = recibirMensaje(socketKernel, mensaje,logCpu);
	switch (header){
			case KRN_TO_CPU_OK:
				log_info(logCpu,"El kernel realizo la operación de WAIT sin bloquear el proceso");
				memset(mensaje, 0, sizeof(t_contenido));
				enviarMensaje(socketKernel,CPU_TO_KRN_OK, mensaje,logCpu);
				break;

			case KRN_TO_CPU_BLOCKED://ver
				log_info(logCpu, "El Kernel me informa que el proceso ha sido bloqueado");
				memset(mensaje, 0, sizeof(t_contenido));
				enviarMensaje(socketKernel,CPU_TO_KRN_OK, mensaje,logCpu);
				quantum = 0;
				endFlag = false;
				bloqueado = true;
				break;

			default:
			;
	}

	/*log_info(logCpu, "wait:: Informa al kernel que ejecute la funcion wait para un semaforo ");*/
}

void signalHandler(t_nombre_semaforo identificador_semaforo){
	t_contenido mensaje;
	memset(mensaje, 0, sizeof(t_contenido));
	strcpy(mensaje,identificador_semaforo);
	enviarMensaje(socketKernel,SYSCALL_SIGNAL_REQUEST, mensaje,logCpu);
	log_info(logCpu, string_from_format("Se envia solicitud de SIGNAL al Kernel para el semaforo:%s",identificador_semaforo));

	/*log_info(logCpu, "signalHandler:: Informa al kernel que ejecute la funcion signal para un semaforo");*/
}


/*
 * Archivo para subir las funciones primitivas que utilizará el libparser.so
 */
AnSISOP_funciones functions = {
	.AnSISOP_definirVariable		 = definirVariable,
	.AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable,
	.AnSISOP_dereferenciar			 = dereferenciar,
	.AnSISOP_asignar				 = asignar,
	.AnSISOP_obtenerValorCompartida  = obtenerValorCompartida,
	.AnSISOP_asignarValorCompartida  = asignarValorCompartida,
	.AnSISOP_irAlLabel               = irAlLabel,
	.AnSISOP_llamarSinRetorno        = llamarSinRetorno,
	.AnSISOP_llamarConRetorno        = llamarConRetorno,
	.AnSISOP_finalizar               = finalizar,
	.AnSISOP_retornar                = retornar,
	.AnSISOP_imprimir				 = imprimir,
	.AnSISOP_imprimirTexto			 = imprimirTexto,
	.AnSISOP_entradaSalida			 = entradaSalida
};

AnSISOP_kernel kernel_functions = {
	.AnSISOP_wait   = wait,
	.AnSISOP_signal = signalHandler
};
