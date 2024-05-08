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

typedef enum{
	MENSAJE,
	PAQUETE
}op_code;

typedef enum{
	CPU,
	CPU_DISPATCH,
	CPU_INTERRUPT,
	IO,
	KERNEL,
	MEMORIA
}modulo_code;

typedef struct{
	int socket_servidor;
	modulo_code modulo;
}t_conexion_escucha;

#endif /* ESTRUCTURAS_CON_H */