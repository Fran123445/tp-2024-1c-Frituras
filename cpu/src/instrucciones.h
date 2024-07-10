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
void enviarDireccionesFisicasAKernel(char *interfaz, registrosCPU registroDireccion, registrosCPU registroTamaño, op_code operacion);
void IO_STDIN_READ(char* interfaz,registrosCPU registroDireccion, registrosCPU registroTamaño);
void IO_STDOUT_WRITE(char* interfaz,registrosCPU registroDireccion, registrosCPU registroTamaño);
void enviar_a_kernel_recurso(char* recurso, op_code cod_op);
void WAIT(char* recurso);
void SIGNAL(char* recurso);
void IO_FS_CREATE(char* interfaz, char* nombre_archivo);
void IO_FS_DELETE(char* interfaz, char* nombre_archivo);
void IO_FS_TRUNCATE(char* interfaz, char* nombre_archivo, registrosCPU registroTam);
void enviar_Direcciones_Fisicas_FS(char* interfaz, char* nombre_archivo, registrosCPU registroDireccion, registrosCPU registroTamaño, registrosCPU registroPunteroArchivo, op_code operacion);
void IO_FS_WRITE(char* interfaz, char* nombre_archivo, registrosCPU registroDireccion, registrosCPU registroTam , registrosCPU registroPunteroArchivo);
void IO_FS_READ(char* interfaz, char* nombre_archivo, registrosCPU registroDireccion, registrosCPU registroTam , registrosCPU registroPunteroArchivo);
void EXIT();


#endif /* CPU_H_ */
