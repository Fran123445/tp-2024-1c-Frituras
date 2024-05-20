#include "instrucciones.h"
#define INSTRUCCION 1 //para verificar si se recibio una instruccion

t_log* logger_cpu;
logger_cpu = log_create(LOG_FILE_PATH, "CPU", true, LOG_LEVEL_INFO);

int recibir_operacion(int socket_cliente) {
    int cod_op;
    if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0) {
        return cod_op;
    } else {
        close(socket_cliente);
        return -1;
    }
}

void fetch(PCB pcb, int socket_memoria){
    uint32_t pc = pcb.programCounter;
    
    t_instruccion instruccionEncontrada = obtener_instruccionDeMemoria(socket_memoria,pc);

    pcb->programCounter++;

    decode_execute(instruccionEncontrada,pcb);
}

void obtener_instruccionDeMemoria(int socket_memoria,uint32_t pc){
    t_paquete* paquete = crear_paquete();
    agregar_a_paquete(paquete, &pc, sizeof(uint32_t));
    enviar_paquete(paquete,socket_memoria);
    eliminar_paquete(paquete);

    int cod_op = recibir_operacion(socket_memoria);
    if (cod_op == INSTRUCCION) { //verificar si se recibio una instruccion
        t_buffer* buffer = recibir_buffer(socket_memoria);
        t_instruccion* instruccion = buffer_read_instruccion(buffer);
        liberar_buffer(buffer);
        return instruccion;
    } else { //caso de que no se recibio bien una instruccion
        log_error(logger_cpu, "Error al recibir instrucción de la memoria");
        return NULL;
    } 
        
}

void decode_execute(t_instruccion instruccion,PCB pcb,t_conexion_escucha* oyente_interrupt){
    switch (instruccion.tipo)
    {
    case SET:
        SET(*(registrosCPU *)instruccion.arg1, *(int *)instruccion.arg2);
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
        SUM(*(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    case SUB:
        SUB(*(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    case JNZ:
        JNZ(*(registrosCPU *)instruccion.arg1, *(int *)instruccion.arg2);
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
        IO_GEN_SLEEP(instruccion.interfaz, *(int *)instruccion.arg1);
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
    /*
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
        log_error(logger_cpu, "La instruccion es inválida");
        break;
    }
    log_info(logger_cpu, "Actualización del Program Counter");
	check_interrupt(pcb,oyente_interrupt);
}

void check_interrupt(PCB pcb){
    //implementar chequeo de interrupciones
    
}
