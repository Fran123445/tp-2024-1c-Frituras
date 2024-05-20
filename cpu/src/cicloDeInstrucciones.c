#include "cicloDeInstruccion.h"


//t_log* logger_cpu = log_create("Cpu.log", "CPU", false, LOG_LEVEL_INFO);


t_instruccion* fetch(PCB* pcb, int socket_memoria){
    uint32_t pc = pcb->programCounter;
    
    enviar_PC_a_memoria(socket_memoria,pc);
    t_instruccion* instruccionEncontrada = obtener_instruccion_de_memoria(socket_memoria);

    pcb->programCounter++;

    return instruccionEncontrada;
}

void enviar_PC_a_memoria(int socket_memoria,uint32_t pc){
    t_paquete* paquete = crear_paquete(ENVIO_PC);
    agregar_a_paquete(paquete, &pc, sizeof(uint32_t));
    enviar_paquete(paquete,socket_memoria);
    eliminar_paquete(paquete);
}

t_instruccion* obtener_instruccion_de_memoria(int socket_memoria){
    op_code cod_op = recibir_operacion(socket_memoria);
    if (cod_op == ENVIO_DE_INSTRUCCIONES) { 
        t_buffer* buffer = recibir_buffer(socket_memoria);
        t_instruccion* instruccion = buffer_read_instruccion(buffer);
        liberar_buffer(buffer);
        return instruccion;
    } else { 
        //log_error(logger_cpu, "Error al recibir instrucción de la memoria");
        return NULL;
    } 
        
}

void decode_execute(t_instruccion* instruccion){
    switch (instruccion->tipo)
    {
    case SET:
        SET(*(registrosCPU *)instruccion->arg1, *(int *)instruccion->arg2);
        break;
    /*
    case MOV_IN:
        MOV_IN(*(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    case MOV_OUT:
        MOV_OUT(*(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    */
    case SUM:
        SUM(*(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);
        break;
    case SUB:
        SUB(*(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);
        break;
    case JNZ:
        JNZ(*(registrosCPU *)instruccion->arg1, *(int *)instruccion->arg2);
        break;
    /*case RESIZE:
        RESIZE(*(int *)instruccion.arg1);
        break;
    case COPY_STRING:
        COPY_STRING(*(int *)instruccion.arg1);
        break;
    case WAIT:
        WAIT(instruccion.interfaz);
        break;
    case SIGNAL:
        SIGNAL(instruccion.interfaz);
        break;
    */
    case IO_GEN_SLEEP:
        IO_GEN_SLEEP(instruccion->interfaz, *(int *)instruccion->arg1);
        break;
    /*
    case IO_STDIN_READ:
        IO_STDIN_READ(instruccion.interfaz, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    case IO_STDOUT_WRITE:
        IO_STDOUT_WRITE(instruccion.interfaz, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    
    case IO_FS_CREATE:
        IO_FS_CREATE(instruccion.interfaz, instruccion.archivo);
        break;
    
    case IO_FS_DELETE:
        IO_FS_DELETE(instruccion.interfaz, instruccion.archivo);
        break;
    case IO_FS_TRUNCATE:
        IO_FS_TRUNCATE(instruccion.interfaz, instruccion.archivo, *(registrosCPU *)instruccion.arg1);
        break;
    case IO_FS_WRITE:
        IO_FS_WRITE(instruccion.interfaz, instruccion.archivo, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2, *(registrosCPU *)instruccion.arg3);
        break;
    case IO_FS_READ:
        IO_FS_READ(instruccion.interfaz, instruccion.archivo, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2, *(registrosCPU *)instruccion.arg3);
        break;
    */
    case EXIT:
        EXIT();
        break;
    default:
        //log_error(logger_cpu, "La instruccion es inválida");
        break;
    }
    //log_info(logger_cpu, "Actualización del Program Counter");
}

//void check_interrupt(PCB pcb){
 //   if(hay_interrupcion){
   //     t_paquete* paquete = crear_paquete(INTERRUPCION);
   //     agregar_a_paquete(paquete, pcb, sizeof(PCB));
   //     enviar_paquete(paquete, socket_kernel);
    //    eliminar_paquete(paquete);
   // }
    
//}
