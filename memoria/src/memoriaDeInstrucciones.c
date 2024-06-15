#include "memoriaDeInstrucciones.h"

t_proceso_memoria* hallar_proceso(int PID){
    bool _mismoPID(t_proceso_memoria* proceso){
        return (proceso->pid == PID);
    };

    pthread_mutex_lock(&mutex_lista_procesos);
    t_proceso_memoria* encontrado = list_find(lista_de_procesos, (void *)_mismoPID);
    pthread_mutex_unlock(&mutex_lista_procesos);

    return encontrado;
}

void destruir_proceso(t_proceso_memoria* proceso){
    if(proceso!=NULL){
        if(proceso->instrucciones != NULL){
            list_destroy_and_destroy_elements(proceso->instrucciones,free); // libera memoria asignada a cada instruccion del proceso y la lista proceso->instrucciones
        }
        if (proceso->tabla_del_proceso != NULL){
           list_destroy_and_destroy_elements(proceso->tabla_del_proceso, free);
        }
        free(proceso->path);
        free(proceso); //libera la memoria asignada al propio proceso
    }
}

void eliminar_proceso_de_lista_de_procesos(int PID){
    bool _mismoPID(t_proceso_memoria* proceso){
        return (proceso->pid == PID);
    };

    pthread_mutex_lock(&mutex_lista_procesos);
    list_remove_and_destroy_by_condition(lista_de_procesos, (void*)_mismoPID, (void*) destruir_proceso);
    pthread_mutex_unlock(&mutex_lista_procesos);
}


char* obtener_instruccion(int socket_kernel, int pc, int pid){
    t_proceso_memoria* proceso = hallar_proceso(pid);
    t_list* lista_instrucciones = proceso->instrucciones;

    char* instruccion_char = list_get(lista_instrucciones, pc);

    return instruccion_char;
}
