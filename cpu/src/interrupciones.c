#include "interrupciones.h"

op_code cod_op_int;
pthread_mutex_t mutexInterrupt;

void recibir_interrupcion(){
    
    cod_op_int = recibir_operacion(socket_kernel_i);
    t_buffer* buffer = recibir_buffer(socket_kernel_i);
    int pid = buffer_read_int(buffer);
    if ((cod_op_int == FINALIZAR_PROCESO && pid == pcb->PID )|| (cod_op_int == FIN_DE_Q && pid == pcb->PID)){ 
        pthread_mutex_lock(&mutexInterrupt);
        hay_interrupcion = 1;
        pthread_mutex_unlock(&mutexInterrupt);
    }
}