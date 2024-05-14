#ifndef SER_H
#define SER_H
#include "pcb.h"
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<netdb.h>

typedef enum {
    PAQUETE,
    // Kernel - IO
    CONECTARIOGENERICA,

    // Kernel - CPU
    //Kernel - Memoria
    CREACION_PROCESO, //Acá Kernel me envia path
    FIN_PROCESO,
    //CPU o IO - Memoria
    ACCESO_ESPACIO_USUARIO,
    //Memoria de Instrucciones - CPU
    ENVIO_DE_INSTRUCCIONES,
    // Cualquier módulo a Memoria
    AJUSTAR_SIZE_PROCESO,
    ACCESO_TABLAS_PAGINAS,

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


t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void* recibir_buffer(int*, int);

#endif /* SER_H */