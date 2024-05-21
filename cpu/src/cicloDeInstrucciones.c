#include "cicloDeInstrucciones.h"


t_log* log_ciclo;
pthread_mutex_t mutexInterrupt;

PCB* recibir_pcb(int socket){
    op_code cod_op = recibir_operacion(socket);
    if(cod_op == ENVIO_PCB){
        t_buffer* buffer = recibir_buffer(socket);
        PCB* pcb= buffer_read_pcb(buffer);
        liberar_buffer(buffer);
        return pcb;
    }
    return NULL;
}

void enviar_pcb(int socket,op_code motivo,PCB* pcb){
    t_paquete* paquete = crear_paquete(motivo);
    agregar_a_paquete(paquete, pcb, sizeof(PCB));
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
}

t_instruccion* fetch(PCB* pcb, int socket_memoria){
    log_ciclo = log_create("Fetch.log", "Instruccion Buscada", false, LOG_LEVEL_INFO);
    int pid = pcb->PID;
    uint32_t pc = pcb->programCounter;

    log_info(log_ciclo, "PID: %u - FETCH - Program Counter: %u", pid, pc);

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
    log_ciclo = log_create("ErrorRecepcionInstruccion.log","Error de recepcion de instruccion",false,LOG_LEVEL_INFO);
    op_code cod_op = recibir_operacion(socket_memoria);
    if (cod_op == ENVIO_DE_INSTRUCCIONES) { 
        t_buffer* buffer = recibir_buffer(socket_memoria);
        t_instruccion* instruccion = buffer_read_instruccion(buffer);
        liberar_buffer(buffer);
        return instruccion;
    } else { 
        log_error(log_ciclo, "Error al recibir instrucción de la memoria");
        return NULL;
    } 
        
}

void decode_execute(t_instruccion* instruccion){
    switch (instruccion->tipo)
    {
    case iSET:
        SET(*(registrosCPU *)instruccion->arg1, *(int *)instruccion->arg2);
        break;
    /*
    case iMOV_IN:
        MOV_IN(*(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    case iMOV_OUT:
        MOV_OUT(*(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    */
    case iSUM:
        SUM(*(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);
        break;
    case iSUB:
        SUB(*(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);
        break;
    case iJNZ:
        JNZ(*(registrosCPU *)instruccion->arg1, *(int *)instruccion->arg2);
        break;
    /*case iRESIZE:
        RESIZE(*(int *)instruccion.arg1);
        break;
    case iCOPY_STRING:
        COPY_STRING(*(int *)instruccion.arg1);
        break;
    case iWAIT:
        WAIT(instruccion.interfaz);
        break;
    case iSIGNAL:
        SIGNAL(instruccion.interfaz);
        break;
    */
    case iIO_GEN_SLEEP:
        IO_GEN_SLEEP(instruccion->interfaz, *(int *)instruccion->arg1);
        break;
    /*
    case iIO_STDIN_READ:
        IO_STDIN_READ(instruccion.interfaz, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    case iIO_STDOUT_WRITE:
        IO_STDOUT_WRITE(instruccion.interfaz, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2);
        break;
    
    case iIO_FS_CREATE:
        IO_FS_CREATE(instruccion.interfaz, instruccion.archivo);
        break;
    
    case iIO_FS_DELETE:
        IO_FS_DELETE(instruccion.interfaz, instruccion.archivo);
        break;
    case iIO_FS_TRUNCATE:
        IO_FS_TRUNCATE(instruccion.interfaz, instruccion.archivo, *(registrosCPU *)instruccion.arg1);
        break;
    case iIO_FS_WRITE:
        IO_FS_WRITE(instruccion.interfaz, instruccion.archivo, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2, *(registrosCPU *)instruccion.arg3);
        break;
    case IO_FS_READ:
        iIO_FS_READ(instruccion.interfaz, instruccion.archivo, *(registrosCPU *)instruccion.arg1, *(registrosCPU *)instruccion.arg2, *(registrosCPU *)instruccion.arg3);
        break;
    */
    case iEXIT:
        EXIT();
        break;
    default:
        //log_error(logger_cpu, "La instruccion es inválida");
        break;
    }
    //log_info(logger_cpu, "Actualización del Program Counter");
}

int check_interrupt(PCB* pcb, int socket_kernel) {
    pthread_mutex_lock(&mutexInterrupt);
    if (hay_interrupcion) {
        hay_interrupcion = 0;
        enviar_pcb(socket_kernel,INTERRUPCION,pcb);
        pthread_mutex_unlock(&mutexInterrupt);
        return 1;
    } else {
        pthread_mutex_unlock(&mutexInterrupt);
        return 0;
    }
}


void realizar_ciclo_de_instruccion(PCB* pcb, int socket_memoria, int socket_kernel){
    while (1) {
        t_instruccion* instruccion_a_ejecutar = fetch(pcb, socket_memoria);
        decode_execute(instruccion_a_ejecutar);
        
        if (check_interrupt(pcb, socket_kernel)) {
            break; // Romper el bucle si hay interrupción
        }
        
        // Verificar condiciones de salida (por ejemplo, instrucción EXIT)
        if (instruccion_a_ejecutar->tipo == iEXIT) {
            enviar_pcb(socket_kernel,INSTRUCCION_EXIT,pcb);
            break; // Romper el bucle si el proceso ha finalizado
        }
    }
}