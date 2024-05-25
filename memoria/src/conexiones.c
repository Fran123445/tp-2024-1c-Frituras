#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <pthread.h>
#include <semaphore.h>
#include "conexiones.h"

pthread_mutex_t mutex_log_memoria_io = PTHREAD_MUTEX_INITIALIZER;
t_log* log_memoria_io;

void esperar_clientes_IO(t_conexion_escucha* nueva_conexion){
    while (1) {
        int* socket_cliente = malloc(sizeof(int));
        pthread_t hilo;
       *socket_cliente = esperar_cliente(nueva_conexion->socket_servidor, nueva_conexion->modulo);
       if(*socket_cliente < 0){
        free(socket_cliente);
        break;
       }
        void (*funcion)(int*);

        switch(recibir_operacion(*socket_cliente)) {
            case CONEXION_STDIN:
                funcion = &funcion_io;
                break;
            case CONEXION_STDOUT:
                // A implementar
                break;
            case CONEXION_DIAL_FS:
            break;

            default:
                pthread_mutex_lock(&mutex_log_memoria_io);
                log_error(log_memoria_io, "Conexión inválida de una interfaz");
                pthread_mutex_unlock(&mutex_log_memoria_io);
                break;
        }
        pthread_create(&hilo,
                        NULL,
                        (void*) funcion,
                        socket_cliente);
        pthread_detach(hilo);
    }
       }
       
void funcion_io(int* socket_cliente){
    // a implementar
}