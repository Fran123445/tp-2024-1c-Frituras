#include "interrupciones.h"

op_code cod_op_int;
pthread_mutex_t mutexInterrupt;

int recibir_interrupcion(){
    
    cod_op_int = recibir_operacion(socket_kernel_i);

    if(cod_op_int <= 0 ){
        return -1;
    }

    t_buffer* buffer = recibir_buffer(socket_kernel_i);
    int pid = buffer_read_int(buffer);

    pthread_mutex_lock(&mutexPCB);
    if(hayPCB != 0){
        if ((cod_op_int == FINALIZAR_PROCESO && pid == pcb->PID ) || 
            (cod_op_int == FIN_DE_Q && pid == pcb->PID)){ 
                pthread_mutex_lock(&mutexInterrupt);
                hay_interrupcion = 1;
                pthread_mutex_unlock(&mutexInterrupt);
        }   
    }
    pthread_mutex_unlock(&mutexPCB);

    liberar_buffer(buffer);
    return 0;
}