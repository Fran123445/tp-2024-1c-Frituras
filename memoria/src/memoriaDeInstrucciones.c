#include "memoriaDeInstrucciones.h"

pthread_mutex_t mutex_lista_de_procesos_con_ins = PTHREAD_MUTEX_INITIALIZER;

t_proceso* hallar_proceso_lista_ins(int PID){
    bool _mismoPID(t_proceso* proceso){
        return (proceso->pid == PID);
    };

    pthread_mutex_lock(&mutex_lista_de_procesos_con_ins);
    t_proceso* encontrado = list_find(lista_de_procesos_con_ins, (void *)_mismoPID);
    pthread_mutex_unlock(&mutex_lista_de_procesos_con_ins);

    return encontrado;
}
void destruir_proceso(t_proceso* proceso){
    if(proceso!=NULL){
        list_destroy_and_destroy_elements(proceso->instrucciones,free); // libera memoria asignada a cada instruccion del proceso y la lista proceso->instrucciones
        free(proceso); //libera la memoria asignada al propio proceso
    }
}
void eliminar_proceso_de_lista_ins(int PID){
    bool _mismoPID(t_proceso* proceso){
        return (proceso->pid == PID);
    };
    pthread_mutex_lock(&mutex_lista_de_procesos_con_ins);
    list_remove_and_destroy_by_condition(lista_de_procesos_con_ins, (void*)_mismoPID, (void*) destruir_proceso);
    pthread_mutex_unlock(&mutex_lista_de_procesos_con_ins);
}

char* obtener_instruccion(int socket_kernel, int pc, int pid){
    t_proceso* proceso = hallar_proceso(pid);
    t_list* lista_instrucciones = proceso->instrucciones;
    char* instruccion_char = list_get(lista_instrucciones, pc);
    //free(lista_instrucciones);
    return instruccion_char;
}
