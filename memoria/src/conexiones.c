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
#include "main.h"

pthread_mutex_t mutex_log_memoria_io = PTHREAD_MUTEX_INITIALIZER;
t_log* log_memoria_io;

void* ejecutar_escribir_memoria(void* args){
    t_parametros_io* parametros = (t_parametros_io*)args;
    escribir_memoria(parametros->socket,parametros->tiempo_retardo,parametros->config);
    free(parametros);
    return NULL;
}

void esperar_clientes_IO(t_conexion_escucha* nueva_conexion, int tiempo_retardo, t_config* config){
    log_memoria_io = log_create("memoria-io.log","Memoria", true, LOG_LEVEL_TRACE);
    while (1) {
        int* socket_cliente = malloc(sizeof(int));
        pthread_t hilo;
       *socket_cliente = esperar_cliente(nueva_conexion->socket_servidor, nueva_conexion->modulo);
       if(*socket_cliente < 0){
        free(socket_cliente);
        break;
       }
        void* (*funcion)(void*); //puntero a funcion q toma arg void y devuelve void (ejecutar_escribir_memoria)
        t_parametros_io* parametros = malloc(sizeof(t_parametros_io));
        parametros->socket=*socket_cliente;
        parametros->tiempo_retardo = tiempo_retardo;
        parametros->config=config;
        
        switch(recibir_operacion(*socket_cliente)) {
            case CONEXION_STDIN:
            while(1){
                funcion = ejecutar_escribir_memoria;
                break;
            }
            case CONEXION_STDOUT:
            //esta solo lee
                break;
            case CONEXION_DIAL_FS:
            op_code code_op = recibir_operacion(*socket_cliente);
            if(code_op == ACCESO_ESPACIO_USUARIO_ESCRITURA){
                funcion = ejecutar_escribir_memoria;
            }else if{
                return; // falta implementar lectura
            }
            
            //falta que leaa
            break;

            default:
                pthread_mutex_lock(&mutex_log_memoria_io);
                log_error(log_memoria_io, "Conexión inválida de una interfaz");
                pthread_mutex_unlock(&mutex_log_memoria_io);
                break;
        }
        pthread_create(&hilo,
                        NULL,
                        funcion,
                        parametros);
        pthread_detach(hilo);
    }
       }
       
