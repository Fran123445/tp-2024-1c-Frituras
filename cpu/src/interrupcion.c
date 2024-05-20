#include "interrupciones.h"

pthread_mutex_t mutexInterrupt;

void recibir_interrupcion(int socket_kernel){
    
    op_code cod_op = recibir_operacion(socket_kernel);
    if (cod_op == INTERRUPCION) { 
        cambiar_variable_interrupcion();
    }
}

void cambiar_variable_interrupcion(void){
    return 1;
}