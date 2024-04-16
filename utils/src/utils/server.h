#ifndef SERVER_H_
#define SERVER_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include"estructurasConexion.h"
#include<pthread.h>

#define MAXCONN 5

extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(char*);
void esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
void* atender_cliente(int*);

void iterator(char* value);

#endif /* SERVER_H_ */
