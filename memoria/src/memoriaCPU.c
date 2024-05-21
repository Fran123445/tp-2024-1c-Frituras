#include <utils/serializacion.h>

int recibir_pc(int socketCPUDispatch){
    op_code cod_op = recibir_operacion(socketCPUDispatch);
    if (cod_op == ENVIO_PC){
        t_buffer* buffer = recibir_buffer(socketCPUDispatch);
        int pc = buffer_read_uint32(buffer);
        liberar_buffer(buffer);
        return pc;
    } 
}


