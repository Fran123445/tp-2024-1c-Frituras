#include <commons/collections/list.h>
#include <utils/serializacion.h>
#include <memoriaKernel.h>
#include <utils/pcb.h>
#include <main.c>
#include <utils/semaforosCompartidos.h>

int recibir_pc(int socket_cpu){
    op_code cod_op = recibir_operacion(socket_cpu);
    if (cod_op == ENVIO_PC){
        t_buffer* buffer = recibir_buffer(socket_cpu);
        int pc = buffer_read_uint32(buffer);
        liberar_buffer(buffer);
        return pc;
    } return -1;
}

void* mandar_instruccion_cpu(int socket_kernel, int socketCPUDispatch){
    t_list *lista_pseudocodigo = abrir_archivo_path(socket_kernel);
    while(1){
        sem_wait(semaforo_pc);
        t_instruccion* instruccion= list_get(lista_pseudocodigo,recibir_pc(socketCPUDispatch));
        t_paquete* paquete = crear_paquete(ENVIO_DE_INSTRUCCIONES);
        agregar_instruccion_a_paquete(paquete,instruccion);
        enviar_paquete(paquete, socketCPUDispatch);        
    }
}


