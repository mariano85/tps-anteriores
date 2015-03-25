/*
 * Primitivas.h
 *
 *  Created on: 08/07/2014
 *      Author: utnso
 */

#ifndef PRIMITIVAS_H_
#define PRIMITIVAS_H_

void finalizar();
t_puntero definirVariable(t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_puntero direccion_variable);
void asignar(t_puntero direccion_variable, t_valor_variable valor);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void irAlLabel(t_nombre_etiqueta etiqueta);
void llamarSinRetorno(t_nombre_etiqueta etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void retornar(t_valor_variable retorno);
void imprimir(t_valor_variable valor);
void imprimirTexto(char* texto);
void entradaSalida(t_nombre_dispositivo dispositivo, int32_t tiempo);
void wait(t_nombre_semaforo identificador_semaforo);
void signalHandler(t_nombre_semaforo identificador_semaforo);


#endif /* PRIMITIVAS_H_ */
