#include <commons/collections/list.h>
#include <utils/serializacion.h>
#include <utils/pcb.h>
#include <unistd.h>
#include <commons/config.h>
#include "memoriaKernel.h"
#include "main.h"
#include "memoriaDeInstrucciones.h"

proceso_cpu* recibir_proceso_cpu(int socket_cpu){
    op_code cod_op = recibir_operacion(socket_cpu);
    proceso_cpu* proceso_de_cpu = malloc(sizeof(proceso_cpu));
    if (cod_op == ENVIO_PC){
        t_buffer* buffer = recibir_buffer(socket_cpu);
        int pid = buffer_read_int(buffer);
        uint32_t pc = buffer_read_uint32(buffer);
        liberar_buffer(buffer);
        proceso_de_cpu->pid = pid;
        proceso_de_cpu->pc= pc;
        return proceso_de_cpu;
    } return NULL;
}

void mandar_instruccion_cpu(int socket_kernel, int socket_cpu, int tiempo_retardo){
    proceso_cpu* proceso_de_cpu = recibir_proceso_cpu(socket_cpu);
    char* instruccion = obtener_instruccion(socket_kernel,(proceso_de_cpu->pc), (proceso_de_cpu->pid));
    t_paquete* paquete = crear_paquete(ENVIO_DE_INSTRUCCIONES);
    agregar_string_a_paquete(paquete,instruccion);
    enviar_paquete(paquete, socket_cpu);
    eliminar_paquete(paquete);
}

