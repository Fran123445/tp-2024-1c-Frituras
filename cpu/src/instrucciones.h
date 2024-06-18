#ifndef CPU_H_
#define CPU_H_

#include "main.h"

void inicializar_registros_cpu();
void* obtenerRegistro(registrosCPU registro);
void SET(registrosCPU registro, int valor);
void SUM(registrosCPU registroDestino, registrosCPU registroOrigen);
void SUB(registrosCPU registroDestino, registrosCPU registroOrigen);
void JNZ(registrosCPU registro, int instruccion);
void RESIZE(uint32_t tamanio_en_bytes);
void IO_GEN_SLEEP(char* interfaz,int unidades_trabajo);
void MOV_IN(registrosCPU registroDatos, registrosCPU registroDireccion);
void MOV_OUT(registrosCPU registroDireccion, registrosCPU registroDatos);
void COPY_STRING(uint32_t tam);
void IO_STDIN_READ(char* interfaz,registrosCPU registroDireccion, registrosCPU registroTamaño);
void IO_STDOUT_WRITE(char* interfaz,registrosCPU registroDireccion, registrosCPU registroTamaño);
void EXIT();


#endif /* CPU_H_ */
