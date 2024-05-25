#include "memoriaDeInstrucciones.h"

pthread_mutex_t mutex_lista_de_procesos_con_ins = PTHREAD_MUTEX_INITIALIZER;

t_proceso* hallar_proceso(int PID){
    bool _mismoPID(t_proceso* proceso){
        return (proceso->pid == PID);
    };

    pthread_mutex_lock(&mutex_lista_de_procesos_con_ins);
    t_proceso* encontrado = list_find(lista_de_procesos_con_ins, (void *)_mismoPID);
    pthread_mutex_unlock(&mutex_lista_de_procesos_con_ins);

    return encontrado;
}

char* obtener_instruccion(int socket_kernel, int pc, int pid){
    t_proceso* proceso = hallar_proceso(pid);
    t_list* lista_instrucciones = proceso->instrucciones;
    char* instruccion_char = list_get(lista_instrucciones, pc);
    //free(lista_instrucciones);
    return instruccion_char;
}