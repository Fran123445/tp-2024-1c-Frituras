#include <utils/serializacion.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include "estructuras.h"
#include "memoriaDeInstrucciones.h"
#include "main.h"

t_list* lista_de_procesos = NULL;

pthread_mutex_t mutex_tablas_paginas = PTHREAD_MUTEX_INITIALIZER;

t_proceso_memoria* creacion_proceso(int socket_kernel) {
    op_code cod_op = recibir_operacion(socket_kernel);
    if(cod_op == CREACION_PROCESO){
        t_proceso_memoria* proceso = malloc(sizeof(proceso));
        t_buffer* buffer = recibir_buffer(socket_kernel);
        int pid_proceso= buffer_read_int(buffer);
        proceso->pid=pid_proceso;
        char* path_proceso = buffer_read_string(buffer);
        proceso->path= path_proceso;
        proceso->tabla_del_proceso = list_create(); // info arranca como lista vacía ya q arranca todo vacío
        proceso->tamanio_proceso = 0;
        proceso->instrucciones = list_create();
        proceso->pc = 0;
        liberar_buffer(buffer);
        list_add(lista_de_procesos,proceso); // guardo en la lista de los procesos el proceso!

        return proceso;
    }
    return NULL;
}

void abrir_archivo_path(int socket_kernel){
    t_proceso_memoria* proceso = creacion_proceso(socket_kernel);
    char* path = (proceso->path);
    if (path == NULL){
        exit(1);
    }
    FILE *file = fopen(path,"r");
    if (file == NULL){
            fprintf(stderr, "Archivo vacio");
            free(path);
        }
    char* linea = NULL;
    size_t tamanio = 0;
    ssize_t leidos;
   
    while ((leidos = getline(&linea, &tamanio,file)) != -1){
        if(linea[leidos - 1] == '\n'){
            linea[leidos - 1] = '\0';
        }

        char *linea_copia = strdup(linea);
        if (linea_copia == NULL){
            perror("Error al copiar linea");
            free(linea);
            fclose(file);
            return;
        }
        list_add(proceso->instrucciones, linea_copia);
    }
    t_paquete* paquete = crear_paquete(PAQUETE); // NO BORRAR! esto es para que conecten bien los módulos 
    enviar_paquete(paquete, socket_kernel);

    free(linea);
    fclose(file);
}

void finalizar_proceso(int socket_kernel){
    op_code cod_op = recibir_operacion(socket_kernel);
    if(cod_op == FIN_PROCESO){
        t_buffer* buffer = recibir_buffer(socket_kernel);
        int pid_proceso= buffer_read_int(buffer);
        eliminar_proceso_de_lista_ins(pid_proceso);
        eliminar_proceso_tabla_pags(pid_proceso);
        liberar_buffer(buffer);
    }

}
