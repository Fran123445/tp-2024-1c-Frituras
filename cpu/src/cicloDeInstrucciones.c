#include "cicloDeInstrucciones.h"
#include <semaphore.h>


t_log* log_ciclo;



PCB* recibir_pcb(){
    op_code cod_op = recibir_operacion(socket_kernel_d);
    if(cod_op == ENVIO_PCB){
        t_buffer* buffer = recibir_buffer(socket_kernel_d);
        PCB* pcb= buffer_read_pcb(buffer);
        liberar_buffer(buffer);
        return pcb;
    }
    return NULL;
}

void enviar_pcb(op_code motivo){
    t_paquete* paquete = crear_paquete(motivo);
    agregar_PCB_a_paquete (paquete, pcb);
    enviar_paquete(paquete, socket_kernel_d);
    eliminar_paquete(paquete);
}

t_instruccion* fetch(){
    log_ciclo = log_create("Cpu.log", "Instruccion Buscada", false, LOG_LEVEL_INFO);
    int pid = pcb->PID;
    uint32_t pc = pcb->programCounter;

    log_info(log_ciclo, "PID: %u - FETCH - Program Counter: %u", pid, pc);

    enviar_PC_a_memoria(pc);
    t_instruccion* instruccionEncontrada = obtener_instruccion_de_memoria();

    pcb->programCounter++;

    log_destroy(log_ciclo);

    return instruccionEncontrada;
}

void enviar_PC_a_memoria(uint32_t pc){
    t_paquete* paquete = crear_paquete(ENVIO_PC);
    agregar_a_paquete(paquete, &pc, sizeof(uint32_t));
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}

t_instruccion* obtener_instruccion_de_memoria(){
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
    log_destroy(log_ciclo);
        
}

void decode_execute(t_instruccion* instruccion){
    log_ciclo = log_create("Cpu.log", "Instruccion Ejecutada", false, LOG_LEVEL_INFO);
    switch (instruccion->tipo)
    {
    case iSET:
        SET(*(registrosCPU *)instruccion->arg1, *(int *)instruccion->arg2);
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p, Parametro 2: %p", pcb->PID, instruccion->tipo, instruccion->arg1, instruccion->arg2);
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
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p, Parametro 2: %p", pcb->PID, instruccion->tipo, instruccion->arg1, instruccion->arg2);
        break;
    case iSUB:
        SUB(*(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p, Parametro 2: %p", pcb->PID, instruccion->tipo, instruccion->arg1, instruccion->arg2);
        break;
    case iJNZ:
        JNZ(*(registrosCPU *)instruccion->arg1, *(int *)instruccion->arg2);
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p, Parametro 2: %p", pcb->PID, instruccion->tipo, instruccion->arg1, instruccion->arg2);
        break;
    /*
    case iRESIZE:
        RESIZE(*(int *)instruccion->arg1);
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p", pcb->PID, instruccion->tipo, instruccion->arg1);
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
        IO_GEN_SLEEP((t_interfaz_generica*)instruccion->interfaz, *(int *)instruccion->arg1);
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - Parametro 1: %p, Parametro 2: %p", pcb->PID, instruccion->tipo, instruccion->interfaz->nombre, instruccion->arg1);        
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
        log_info(log_ciclo, "PID: %u - Ejecutando: %u - No tiene parámetros", pcb->PID, instruccion->tipo);
        break;
    default:
        log_error(log_ciclo, "La instruccion es inválida");
        break;
    }
    log_destroy(log_ciclo);
}

int check_interrupt() {
    pthread_mutex_lock(&mutexInterrupt);
    if (hay_interrupcion) {
        hay_interrupcion = 0;
        enviar_pcb(INTERRUPCION);
        pthread_mutex_unlock(&mutexInterrupt);
        return 1;
    } else {
        pthread_mutex_unlock(&mutexInterrupt);
        return 0;
    }
}


void realizar_ciclo_de_instruccion(){
    while (1) {
        t_instruccion* instruccion_a_ejecutar = fetch(pcb, socket_memoria);
        decode_execute(instruccion_a_ejecutar);
        
        if (check_interrupt()) {
            enviar_pcb(INTERRUPCION);
            break; // Romper el bucle si hay interrupción
        }
        // Verificar condiciones de salida 
        if (instruccion_a_ejecutar->tipo == iEXIT){
            break; // Romper el bucle si el proceso ha finalizado
        }
    }
}