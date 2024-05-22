#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <pthread.h>
#include <semaphore.h>
#include<memoriaCPU.h>
#include<memoriaKernel.h>
#include <utils/serializacion.h>

void* esperar_clientes(t_conexion_escucha* nueva_conexion){
    while (1) {
        int* socket_cliente = malloc(sizeof(int));
        pthread_t hilo;
       *socket_cliente = esperar_cliente(nueva_conexion->socket_servidor, nueva_conexion->modulo);
       if(*socket_cliente < 0){
        free(socket_cliente);
        break;
       }
       }
       return NULL;
    }
