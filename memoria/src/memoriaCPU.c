#include <commons/collections/list.h>
#include <utils/serializacion.h>
#include <utils/pcb.h>
#include <unistd.h>
#include <commons/config.h>
#include "memoriaKernel.h"
#include "main.h"
#include "memoriaInstrucciones.h"

proceso_cpu* recibir_proceso_cpu(int socket_cpu){
    op_code cod_op = recibir_operacion(socket_cpu);
    proceso_cpu* proceso_de_cpu = malloc(sizeof(proceso_cpu));
    if (cod_op == ENVIO_PC){
        t_buffer* buffer = recibir_buffer(socket_cpu);
        uint32_t pc = buffer_read_uint32(buffer);
        int pid = buffer_read_int(buffer);
        liberar_buffer(buffer);
        proceso_de_cpu->pid = pid;
        proceso_de_cpu->pc= pc;
        return proceso_de_cpu;
    } return NULL;
}

void* mandar_instruccion_cpu(int socket_kernel, int socket_cpu, int tiempo_retardo){   
   while(true){ 
    proceso_cpu* proceso_de_cpu = recibir_proceso_cpu(socket_cpu);
    if(proceso_de_cpu == NULL){
        break;
    }
    t_proceso* proceso_a_mandar = hallar_proceso(proceso_de_cpu->pid);
    list_get(proceso_a_mandar->instrucciones,(proceso_de_cpu->pc));
    t_instruccion* instruccion = obtener_instruccion(socket_kernel,(proceso_de_cpu->pc), (proceso_de_cpu->pid));
    usleep(tiempo_retardo*1000);
    t_paquete* paquete = crear_paquete(ENVIO_DE_INSTRUCCIONES);
    agregar_instruccion_a_paquete(paquete,instruccion);
    enviar_paquete(paquete, socket_cpu);   
    eliminar_paquete(paquete);     
    }
}


