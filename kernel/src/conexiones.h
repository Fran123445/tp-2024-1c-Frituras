#ifndef CONN_H
#define CONN_H

#include <stdlib.h>
#include <stdio.h>
#include <threads.h>
#include <utils/server.h>
#include <procesos.h>
#include <semaphore.h>

void esperarClientesIO(t_conexion_escucha* params);
void administrarConexionIO(int socket_cliente);

#endif /* CONN_H */