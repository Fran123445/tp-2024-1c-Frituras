#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <pthread.h>
#include <semaphore.h>
#include "accesoEspacioUsuario.h"
#include "conexiones.h"

pthread_mutex_t mutex_log_memoria_io = PTHREAD_MUTEX_INITIALIZER;
t_log* log_memoria_io;

// Funciones auxiliares requeridas para pasarle como parámetro al hilo

void* ejecutar_leer_memoria(void* arg) {
    int socket = *(int*)arg;
    free(arg);
    return leer_memoria(socket);
}

void* ejecutar_escribir_memoria(void* arg) {
    int socket = *(int*)arg;
    free(arg);
    return escribir_memoria(socket);
}

// Función para esperar los clientes

void esperar_clientes_IO(t_conexion_escucha* nueva_conexion){

    log_memoria_io = log_create("memoria-io.log","Memoria", true, LOG_LEVEL_TRACE);

    while (1) {
        int* socket_cliente = malloc(sizeof(int));
        pthread_t hilo;
        *socket_cliente = esperar_cliente(nueva_conexion->socket_servidor, nueva_conexion->modulo);
        if(*socket_cliente < 0){
            free(socket_cliente);
            break;
        }

        void* (*funcion)(void*); //puntero a funcion q toma arg void y devuelve void
        op_code code_op;

        switch(recibir_operacion(*socket_cliente)){
            case CONEXION_STDIN:
                while(1){
                    code_op = recibir_operacion(*socket_cliente);
                    if(code_op == ACCESO_ESPACIO_USUARIO_ESCRITURA){
                        funcion = ejecutar_escribir_memoria;
                    }
                break;
            }
            case CONEXION_STDOUT:
                code_op = recibir_operacion(*socket_cliente);
                if(code_op == ACCESO_ESPACIO_USUARIO_LECTURA){
                    funcion = ejecutar_leer_memoria;
                }
                break;
            case CONEXION_DIAL_FS:
                code_op = recibir_operacion(*socket_cliente);
                if(code_op == ACCESO_ESPACIO_USUARIO_ESCRITURA){
                    funcion = ejecutar_escribir_memoria;
                }else if(code_op == ACCESO_ESPACIO_USUARIO_LECTURA){
                    funcion = ejecutar_leer_memoria;
                }            
                break;

            default:
                pthread_mutex_lock(&mutex_log_memoria_io);
                log_error(log_memoria_io, "Conexión inválida de una interfaz");
                pthread_mutex_unlock(&mutex_log_memoria_io);
                break;
        }

        pthread_create(&hilo, NULL, funcion, (void*)socket_cliente);
        pthread_detach(hilo);
    }
}
       
