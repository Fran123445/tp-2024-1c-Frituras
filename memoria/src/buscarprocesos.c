#include "buscarprocesos.h"

t_proceso* hallar_proceso(int PID){
    bool _mismoPID(t_proceso* proceso){
        return (proceso->pid == PID);
    };

    pthread_mutex_lock(&mutex_lista_procesos);
    t_proceso* encontrado = list_find(lista_de_procesos_con_ins, (*void)_mismoPID);
    pthread_mutex_unlock(&mutex_lista_procesos);

    return encontrado;
}