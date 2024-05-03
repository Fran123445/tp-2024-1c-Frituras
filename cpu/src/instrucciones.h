#ifndef CPU_H_
#define CPU_H_

#include <stddef.h> // Agrega esta línea si usas size_t
#include <stdio.h>
#include "registroCpu.h"


// Prototipos de funciones
void inicializar_registros_cpu();
void* obtenerRegistro(registrosCPU registro);
size_t tamanioRegistro(registrosCPU registro);
void SET(registrosCPU registro, int valor);
void SUM(registrosCPU registroDestino, registrosCPU registroOrigen);
void SUB(registrosCPU registroDestino, registrosCPU registroOrigen);
void JNZ(registrosCPU registro, int instruccion);



#endif /* CPU_H_ */
