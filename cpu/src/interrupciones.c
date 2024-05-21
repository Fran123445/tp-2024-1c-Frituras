#include "interrupciones.h"

pthread_mutex_t mutexInterrupt;

void recibir_interrupcion(){
    
    op_code cod_op = recibir_operacion(socket_kernel_i);
    if (cod_op == INTERRUPCION) { 
        pthread_mutex_lock(&mutexInterrupt);
        hay_interrupcion = 1;
        pthread_mutex_unlock(&mutexInterrupt);
    }
}