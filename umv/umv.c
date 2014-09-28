/*
 * umv_prueba.c
 *
 *  Created on: 02/05/2014
 *      Author: utnso
 */

#include "umv.h"

int main(){

	// en esta funcion se levantan los logs, la configuracion y se crea la lista de segmentos
	iniciarUMV();

	// se mandan por parametro el pid y los tamanios de los 4 segmentos
	t_segmento_plp *msg_plp1 = crearSegmentoPLP(1, 800, 800, 800, 800);
	t_segmento_plp *msg_plp2 = crearSegmentoPLP(2, 800, 800, 800, 800);

	// esto en vez de allocarlos deberia recibirlos directamente del PLP
	crearSegmentos(msg_plp1);
	crearSegmentos(msg_plp2);

	void _imprimirSegmentoUMV(t_segmento_umv *nodo){
		printf("\nimprimo el segmento: %s. PID: %d, dir fisica: %d. tamanio %d. disponible? %s\n"
			, TIPO_SEGMENTO_NOMBRES[nodo->tipo]
			, nodo->pid
			, nodo->direccionFisica
			, nodo->tamanio
			, BOOL_NOMBRE[nodo->disponible]);
	}

	inhabilitarSegmentos(msg_plp1->pid);
	list_iterate(listaSegmentosUMV, (void*)_imprimirSegmentoUMV);

	destruirSegmentos(msg_plp1->pid);
	puts("DESTRUTO LOS SEGMENTOS");
	list_iterate(listaSegmentosUMV, (void*)_imprimirSegmentoUMV);

	compactacion();

	finalizarUMV();
	free(msg_plp1);
	free(msg_plp2);

	return EXIT_SUCCESS;
}

void _destruirSegmento(t_segmento_umv *segmento){
	free(segmento);
}

int32_t getTamanioSegmentoFromBase(uint32_t base){

	bool _buscar_x_dir_base(t_segmento_umv *p) {
		return (p->direccionFisica == base);
	}

	t_segmento_umv * nodo = list_find(listaSegmentosUMV, (void*)_buscar_x_dir_base);

	return nodo == NULL ? -1 : nodo->tamanio;
}

bool validarOperacionBasica(uint32_t base, uint32_t offset, uint32_t tamanio, int32_t tamanioSegmento){

	if(tamanioSegmento == -1){
		puts("puteamos porque no encontro el segmento de esa base");
		return false;
	}

	if( (base + offset + tamanio) > tamanioSegmento){
		puts("tambien puteamos porque nos pasamos del segmento");
		return false;
	}

	return true;
}

char* solicitarBytes(uint32_t base, uint32_t offset, uint32_t tamanio){
	char *buffer;

	if(!validarOperacionBasica(base, offset, tamanio, getTamanioSegmentoFromBase(base))){
		return NULL;
	}

	buffer = calloc(1, tamanio); // reservo memoria
	char *puntero_UMV = UMV_PTR + base + offset; // apunto a la UMV

	memcpy(buffer, puntero_UMV, tamanio); // copio el buffer
	return buffer;
}

bool escribirBytes(uint32_t base, uint32_t offset, uint32_t tamanio, char* buffer){

	if(!validarOperacionBasica(base, offset, tamanio, getTamanioSegmentoFromBase(base))){
		return false;
	}

	char *puntero_UMV = UMV_PTR + base + offset; // apunto a la UMV
	memcpy(puntero_UMV, buffer, tamanio);

	return true;
}

//void crearSegmento(int32_t pid, uint32_t tamanio){
//
//	int32_t direccionLibre() = obtenerDireccionLibre(tamanio);
//
//}

void compactacion(){
	bool _noEstaDisponible(t_segmento_umv *p) {
		return (p->disponible == false);
	}

	t_list *listaEliminados = list_filter(listaSegmentosUMV, (void*)_noEstaDisponible);
	list_remove_and_destroy_by_condition(listaSegmentosUMV, (void*)_noEstaDisponible, (void*)_destruirSegmento);
	list_destroy(listaEliminados);
}

// TODO: o se crean los 4 segmentos, o no se crea ninguno y marco las direcciones con -1
void crearSegmentos(t_segmento_plp *msg_plp){

	// generamos un nuevo nodo
	t_segmento_umv *segUMV1, *segUMV2, *segUMV3, *segUMV4;

	segUMV1 = calloc(1, sizeof(t_segmento_umv));
	segUMV2 = calloc(1, sizeof(t_segmento_umv));
	segUMV3 = calloc(1, sizeof(t_segmento_umv));
	segUMV4 = calloc(1, sizeof(t_segmento_umv));

	segUMV1->pid = msg_plp->pid;
	segUMV2->pid = msg_plp->pid;
	segUMV3->pid = msg_plp->pid;
	segUMV4->pid = msg_plp->pid;

	segUMV1->tipo = SEGMENTO_CODIGO_LITERAL;
	segUMV2->tipo = SEGMENTO_INDICE_DE_ETIQUETAS;
	segUMV3->tipo = SEGMENTO_INDICE_DE_FUNCIONES;
	segUMV4->tipo = SEGMENTO_STACK;

	segUMV1->tamanio = msg_plp->tamanio[SEGMENTO_CODIGO_LITERAL];
	segUMV2->tamanio = msg_plp->tamanio[SEGMENTO_INDICE_DE_ETIQUETAS];
	segUMV3->tamanio = msg_plp->tamanio[SEGMENTO_INDICE_DE_FUNCIONES];
	segUMV4->tamanio = msg_plp->tamanio[SEGMENTO_STACK];

	segUMV1->disponible = true;
	segUMV2->disponible = true;
	segUMV3->disponible = true;
	segUMV4->disponible = true;

	// hay que validar que haya memoria?
	segUMV1->direccionFisica = obtenerDireccionLibre();
	list_add(listaSegmentosUMV, segUMV1);

	segUMV2->direccionFisica = obtenerDireccionLibre();
	list_add(listaSegmentosUMV, segUMV2);

	segUMV3->direccionFisica = obtenerDireccionLibre();
	list_add(listaSegmentosUMV, segUMV3);

	segUMV4->direccionFisica = obtenerDireccionLibre();
	list_add(listaSegmentosUMV, segUMV4);

	msg_plp->direccionFisica[SEGMENTO_CODIGO_LITERAL] = segUMV1->direccionFisica;
	msg_plp->direccionFisica[SEGMENTO_INDICE_DE_ETIQUETAS] = segUMV2->direccionFisica;
	msg_plp->direccionFisica[SEGMENTO_INDICE_DE_FUNCIONES] = segUMV3->direccionFisica;
	msg_plp->direccionFisica[SEGMENTO_STACK] = segUMV4->direccionFisica;
}

t_segmento_plp* crearSegmentoPLP(int32_t pid
		, int32_t tam1
		, int32_t tam2
		, int32_t tam3
		, int32_t tam4)
{

	t_segmento_plp *msg_plp;
	msg_plp = calloc(1, sizeof(t_segmento_plp));

	msg_plp->pid = pid;

	// lo que me importa recibir por ahora son los tamanios de los segmentos
	// y lo que me importa devolver es este mismo mensaje con las direcciones fisicas asignadas
	msg_plp->tamanio[SEGMENTO_CODIGO_LITERAL] = tam1;
	msg_plp->tamanio[SEGMENTO_INDICE_DE_ETIQUETAS] = tam2;
	msg_plp->tamanio[SEGMENTO_INDICE_DE_FUNCIONES] = tam3;
	msg_plp->tamanio[SEGMENTO_STACK] = tam4;

	return msg_plp;
}

void inhabilitarSegmentos(int32_t pid){
	// la lista SIEMPRE tiene que tener 4 elementos
	void _inhabilitarSegmento(t_segmento_umv *p){
		if(p->pid == pid){
			p->disponible = false;
		}
	}

	list_iterate(listaSegmentosUMV, (void*)_inhabilitarSegmento);
}

void destruirSegmentos(int32_t pid){

	bool _buscar_x_pid(t_segmento_umv *p) {
		return (p->pid == pid);
	}

	t_list * listaAux = list_filter(listaSegmentosUMV, (void*)_buscar_x_pid);
	uint32_t i, size = list_size(listaAux);

	if(size < 4){
		puts("RECONTRA ERROR!! SIEMPRE TIENE QUE HABER 4 SEGMENTOS POR PRGRAMA!!!");
		exit(-1);
	}

	for(i = 0; i < list_size(listaAux); i++){
		list_remove_by_condition(listaSegmentosUMV, (void*)_buscar_x_pid);
	}

	list_destroy_and_destroy_elements(listaAux, (void*)_destruirSegmento);
}

void iniciarUMV(){
	ALGORITMO_UBICACION = WORST_FIT;

	t_config * config = config_create(UMV_CONF_PATH);

	if (config->properties->elements_amount == 0) {
		printf("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION %s \n", UMV_CONF_PATH);
		perror("\nERROR AL LEVANTAR ARCHIVO DE CONFIGURACION\n( Don't PANIC! Si estas por consola ejecuta: ln -s ../umv.conf umv.conf )\n\n");
		config_destroy(config);
		exit(-1);
	}

	if(!config_has_property(config, "umv_size")){
		puts("FALTA LA PROPERTY 'umv_size");
		config_destroy(config);
		exit(-1);
	}

	UMV_SIZE = config_get_int_value(config, "umv_size");
	config_destroy(config);

	UMV_PTR = (char*)calloc(1, UMV_SIZE);

	LOGGER = log_create(UMV_LOG_PATH, "Programa", true, LOG_LEVEL_DEBUG);
	log_info(LOGGER, "INICIALIZANDO UMV. TAMANIO: '%d' ", UMV_SIZE);

	listaSegmentosUMV = list_create();
}

void finalizarUMV(){
	list_destroy_and_destroy_elements(listaSegmentosUMV, (void*)_destruirSegmento);
	log_destroy(LOGGER);
	free(UMV_PTR);
}

bool compararSegmentos(t_segmento_umv* segmento1, t_segmento_umv* segmento2){
	return segmento1->direccionFisica < segmento2->direccionFisica || segmento1->tamanio == 0;
}

t_segmento_umv *getUltimoSegmento(){
	return (t_segmento_umv *)list_get(listaSegmentosUMV, listaSegmentosUMV->elements_count - 1);
}

// este por ahora es el worst fit. genera fragmentacion
// TODO: deberia recibir por parametro el tamaño el segmento
int32_t obtenerDireccionLibre(int32_t tamanio){
	if(list_is_empty(listaSegmentosUMV)){
		return 0;
	}

	t_segmento_umv *ultimoSegmento = getUltimoSegmento();
//	printf("\nnombre del ultimo segmento: %d - %s\n", ultimoSegmento->pid, TIPO_SEGMENTO_NOMBRES[ultimoSegmento->tipo]);
	int32_t dirLibre = ultimoSegmento->direccionFisica + ultimoSegmento->tamanio;

	return dirLibre < UMV_SIZE ? dirLibre : -1;
}

// TODO: mejorar esta funcion y evaluar si sirve realmente. sino descartarla
int32_t obtenerMemoriaDisponible(){
	t_segmento_umv *ultimoSegmento = getUltimoSegmento();
	return UMV_SIZE - (ultimoSegmento->direccionFisica + ultimoSegmento->tamanio);
}

