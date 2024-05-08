#ifndef SERVER_H_
#define SERVER_H_

#include"estructurasConexion.h"
#include <commons/log.h>
#include<assert.h>
#include "handshake.h"

#define MAXCONN 5

extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(char* puerto, t_log* logger);
int esperar_cliente(int socket_servidor, modulo_code modulo);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
void* atender_cliente(int);

void iterator(char* value);

#endif /* SERVER_H_ */
