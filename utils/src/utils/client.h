#ifndef CLIENT_H_
#define CLIENT_H_

#include"estructurasConexion.h"
#include<signal.h>

int crear_conexion(char* ip, char* puerto, modulo_code modulo);
void liberar_conexion(int socket_cliente);

#endif /* CLIENT_H_ */