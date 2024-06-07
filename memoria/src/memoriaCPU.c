#include <commons/collections/list.h>
#include <utils/serializacion.h>
#include <utils/pcb.h>
#include <unistd.h>
#include <commons/config.h>
#include "memoriaKernel.h"
#include "main.h"
#include "memoriaDeInstrucciones.h"

t_proceso* recibir_proceso_cpu(int socket_cpu){
    op_code cod_op = recibir_operacion(socket_cpu);
    t_proceso* proceso_cpu = malloc(sizeof(proceso_cpu));
    if (cod_op == ENVIO_PC){
        t_buffer* buffer = recibir_buffer(socket_cpu);
        int pid = buffer_read_int(buffer);
        uint32_t pc = buffer_read_uint32(buffer);
        liberar_buffer(buffer);
        proceso_cpu->pid = pid;
        proceso_cpu->pc= pc;
        return proceso_cpu;
    } return NULL;
}

void mandar_instruccion_cpu(int socket_kernel, int socket_cpu, int tiempo_retardo){
    t_proceso* proceso = recibir_proceso_cpu(socket_cpu);
    char* instruccion = obtener_instruccion(socket_kernel,(proceso->pc), (proceso->pid));
    t_paquete* paquete = crear_paquete(ENVIO_DE_INSTRUCCIONES);
    agregar_string_a_paquete(paquete,instruccion);
    enviar_paquete(paquete, socket_cpu);
    eliminar_paquete(paquete);
}

void resize_proceso(int socket_cpu){
    op_code cod_op = recibir_operacion(socket_cpu);
    t_proceso* proceso = malloc(sizeof(proceso));
    if (cod_op == ENVIO_RESIZE){
        t_buffer* buffer = recibir_buffer(socket_cpu);
        int pid = buffer_read_int(buffer);
        proceso->pid = pid;
        int tamanio = buffer_read_int(buffer);
        proceso->tamanio_proceso= tamanio;

}