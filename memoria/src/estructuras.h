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
    int proceso_id;
    char* path;
}proceso_memoria;

extern t_list* lista_de_procesos_con_ins; //adentro tiene t_procesos.
typedef struct{
    int pid;
    t_list* instrucciones;
}t_proceso;

typedef struct{
    int pid;
    uint32_t pc;
}proceso_cpu;

typedef struct{
    int marco;
    bool validez;
}informacion_de_tabla;

typedef struct{
    int pid;
    t_list* tabla_de_paginas; // dentro de él va a estar la estructura informacion_de_tabla
}t_tabla_paginas;
// o sea va a ser una estructura que relacione el PID con la tabla de página y ahí sé que proceso tiene
//qué marcos y qué paginas.

#endif /* ESTRUCTURAS_CON_MEM */