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

t_list* lista_de_procesos_con_ins; //adentro tiene t_procesos.

typedef struct{
    int pid;
    t_list* instrucciones;
}t_proceso;

typedef struct{
    int pid;
    uint32_t pc;
}proceso_cpu;
#endif /* ESTRUCTURAS_CON_MEM */