#ifndef CPU_H_
#define CPU_H_

#include <stddef.h> // Agrega esta línea si usas size_t
#include <stdio.h>
#include "registroCpu.h"


// Prototipos de funciones
void* obtenerRegistro(registrosCPU);
size_t tamanioRegistro(registrosCPU);
void SET(registrosCPU registro, int valor);
void SUM(registrosCPU registroDestino, registrosCPU registroOrigen);
void SUB(registrosCPU registroDestino, registrosCPU registroOrigen);
void JNZ(registrosCPU registro, int instruccion);

// Declaración de la variable global
extern void* lista_de_registros[11];

#endif /* CPU_H_ */
