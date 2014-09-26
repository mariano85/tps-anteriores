/*
 * msp.h
 *
 *  Created on: 26/09/2014
 *      Author: utnso
 */

#ifndef MSP_H_
#define MSP_H_




//Esta estructura me sirve para guardar todos los parametros de configuracion
typedef struct
{
	int puerto;
	int cantidad_memoria;
	int cantidad_swap;
	char* sust_pags;
} t_configuracion;


t_configuracion configuracion;





t_configuracion levantarArchivoDeConfiguracion();



#endif /* MSP_H_ */
