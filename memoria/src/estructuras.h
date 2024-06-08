#ifndef ESTRUCTURAS_CON_MEM
#define ESTRUCTURAS_CON_MEM

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<commons/log.h>
#include<commons/config.h>
#include<netdb.h>
#include<string.h>
#include<pthread.h>
#include <utils/serializacion.h>

typedef struct{
    int marco;
    bool validez;
}informacion_de_tabla;
typedef struct{
    int pid;
    char* path;
    t_list* tabla_del_proceso; //contiene la estructura informacion_de_tabla
    int tamanio_proceso;
    t_list* instrucciones;
    uint32_t pc;
}t_proceso_memoria;
// o sea va a ser una estructura que relacione PID con tabla de página y path. Ahí sé que páginas y marcos tiene ese proceso

extern t_list* lista_de_procesos_con_ins; //adentro tiene t_procesos.
extern t_list* tablas_de_paginas; //lista de proceso_memoria


#endif /* ESTRUCTURAS_CON_MEM */