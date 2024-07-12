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

void manejoSTDOUT(int* socket_cliente) {
    while(1) {
        if (recibir_operacion(*socket_cliente) > 0) {
            leer_memoria(*socket_cliente);
        } else {
            break;
        }
    }
    free(socket_cliente);
    pthread_exit(NULL);
}

void manejoSTDIN(int* socket_cliente) {
    while(1) {
        if (recibir_operacion(*socket_cliente) > 0) {
            escribir_memoria(*socket_cliente);
        } else {
            break;
        }
    }
    free(socket_cliente);
    pthread_exit(NULL);
}

void manejoDIALFS(int* socket_cliente) {
    while(1) {
        op_code code_op = recibir_operacion(*socket_cliente);
        if(code_op == ACCESO_ESPACIO_USUARIO_ESCRITURA){
            escribir_memoria(*socket_cliente);
        }else {
            leer_memoria(*socket_cliente);
        }       
    }
    free(socket_cliente);
    pthread_exit(NULL);
}

// Función para esperar los clientes

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

        switch(recibir_operacion(*socket_cliente)){
            case CONEXION_STDIN:
                funcion = manejoSTDIN;  
                break;
            case CONEXION_STDOUT:
                funcion = manejoSTDOUT;
                break;
            case CONEXION_DIAL_FS:
                funcion = manejoDIALFS;     
                break;
            default:
                log_error(log_memoria, "Conexión inválida de una interfaz");
                free(socket_cliente);
                continue;
        }

        pthread_create(&hilo, NULL, (void *) funcion, (void*)socket_cliente);
        pthread_detach(hilo);
    }
}
  