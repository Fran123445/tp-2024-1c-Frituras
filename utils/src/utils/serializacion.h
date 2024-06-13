#ifndef SER_H
#define SER_H
#include "pcb.h"
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<netdb.h>
#include <unistd.h>
#include <commons/string.h>

typedef enum {
    PAQUETE,
    // Kernel - IO
    CONEXION_IOGENERICA,
    CONEXION_STDIN,
    CONEXION_STDOUT,
    OPERACION_FINALIZADA,
    // Kernel - CPU
    ENVIO_PCB,
    //Bidireccional Kernel - CPU
    INTERRUPCION,
    // CPU - KERNEL
    INSTRUCCION_EXIT,
    INSTRUCCION_WAIT,
    INSTRUCCION_SIGNAL,
    ENVIAR_IO_GEN_SLEEP,
    // CPU - Memoria
    ENVIO_PC,
    ENVIO_RESIZE,
    //Kernel - Memoria
    CREACION_PROCESO, //Acá Kernel me envia path
    FIN_PROCESO,
    //CPU o IO - Memoria
    ACCESO_ESPACIO_USUARIO_LECTURA,
    ACCESO_ESPACIO_USUARIO_ESCRITURA,
    ESCRITURA_REALIZADA_OK,
    CONEXION_DIAL_FS,
    //Memoria de Instrucciones - CPU
    ENVIO_DE_INSTRUCCIONES,
    // Cualquier módulo a Memoria
    AJUSTAR_SIZE_PROCESO,
    ACCESO_TABLAS_PAGINAS,
    OUT_OF_MEMORY,
    RESIZE_ACEPTADO,

}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


t_paquete* crear_paquete(op_code operacion);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
t_buffer* recibir_buffer(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
int recibir_operacion(int socket_cliente);
void agregar_uint32_a_paquete(t_paquete* paquete, uint32_t valor);
void agregar_int_a_paquete(t_paquete* paquete, int valor);
void agregar_string_a_paquete(t_paquete* paquete, char* string);
void agregar_PCB_a_paquete(t_paquete* paquete, PCB* pcb);
void agregar_instruccion_a_paquete(t_paquete* paquete, t_instruccion* instruccion);
void agregar_interfaz_generica_a_paquete(t_paquete* paquete, t_interfaz_generica* interfaz);

int buffer_read_int(t_buffer* buffer);
void buffer_read(t_buffer* buffer, void* data);
uint32_t buffer_read_uint32(t_buffer* buffer);
int read_buffer_tamanio (t_buffer* buffer);
char* buffer_read_string(t_buffer* buffer);
PCB* buffer_read_pcb(t_buffer* buffer);
t_instruccion* buffer_read_instruccion(t_buffer* buffer);
t_interfaz_generica* buffer_read_interfaz_generica(t_buffer* buffer);
void liberar_buffer(t_buffer* buffer);

#endif /* SER_H */
