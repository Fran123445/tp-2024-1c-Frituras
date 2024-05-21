#ifndef CPU_H_
#define CPU_H_

#include <stddef.h> 
#include <stdio.h>
#include <utils/pcb.h>
#include <utils/serializacion.h>
#include "main.h"


void inicializar_registros_cpu();
void* obtenerRegistro(registrosCPU registro);
size_t tamanioRegistro(registrosCPU registro);
void SET(registrosCPU registro, int valor);
void SUM(registrosCPU registroDestino, registrosCPU registroOrigen);
void SUB(registrosCPU registroDestino, registrosCPU registroOrigen);
void JNZ(registrosCPU registro, int instruccion);
void IO_GEN_SLEEP(char* interfaz,int valor);
void EXIT();


#endif /* CPU_H_ */
