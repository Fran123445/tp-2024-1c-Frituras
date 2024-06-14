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
        void* (*funcion)(void*); //puntero a funcion q toma arg void y devuelve void (ejecutar_escribir_memoria)
        
        switch(recibir_operacion(*socket_cliente)) {
            case CONEXION_STDIN:
            while(1){
                recibir_operacion(ACCESO_ESPACIO_USUARIO_ESCRITURA);
                funcion = escribir_memoria;
                break;
            }
            case CONEXION_STDOUT:
            //esta solo lee
                break;
            case CONEXION_DIAL_FS:
            op_code code_op = recibir_operacion(*socket_cliente);
            if(code_op == ACCESO_ESPACIO_USUARIO_ESCRITURA){
                funcion = escribir_memoria;
            }else{
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
                        (void*)socket_cliente);
        pthread_detach(hilo);
    }
}
       
