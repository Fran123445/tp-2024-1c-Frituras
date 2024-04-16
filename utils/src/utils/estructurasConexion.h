#ifndef ESTRUCTURAS_CON_H
#define ESTRUCTURAS_CON_H

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<commons/log.h>
#include<commons/config.h>
#include<netdb.h>
#include<string.h>
#include<pthread.h>

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct 
{
	t_config* config;
	char* ip;
	char* puerto;
}t_conexion;


#endif /* ESTRUCTURAS_CON_H */