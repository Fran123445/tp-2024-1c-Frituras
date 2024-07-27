#include "cicloDeInstrucciones.h"
#include "interrupciones.h"
#include "instrucciones.h"
#include <semaphore.h>

// Funciones auxiliares

registrosCPU string_a_registro(const char* registro) {
    if (strcmp(registro, "PC") == 0) return PC;
    if (strcmp(registro, "AX") == 0) return AX;
    if (strcmp(registro, "BX") == 0) return BX;
    if (strcmp(registro, "CX") == 0) return CX;
    if (strcmp(registro, "DX") == 0) return DX;
    if (strcmp(registro, "EAX") == 0) return EAX;
    if (strcmp(registro, "EBX") == 0) return EBX;
    if (strcmp(registro, "ECX") == 0) return ECX;
    if (strcmp(registro, "EDX") == 0) return EDX;
    if (strcmp(registro, "SI") == 0) return SI;
    if (strcmp(registro, "DI") == 0) return DI;
    // Error si registro no valido
    printf("Error: Registro '%s' no válido.\n", registro);
    exit(1);
}

char* registro_a_string(registrosCPU registro) {
    switch(registro){
        case AX:
            return "AX";
            break;
        case BX:
            return "BX";
            break;
        case CX:
            return "CX";
            break;
        case DX:
            return "DX";
            break;
        case EAX:
            return "EAX";
            break;
        case EBX:
            return "EBX";
            break;
        case ECX:
            return "ECX";
            break;
        case EDX:
            return "EDX";
            break;
        case SI:
            return "SI";
            break;
        case DI:
            return "DI";
            break;
        case PC:
            return "PC";
            break;
        default:
            return "error";
            break;
    }
}

t_tipoInstruccion string_a_tipo_instruccion (char* ins_char){
    if (strcmp(ins_char, "SET") == 0) return iSET;
    if (strcmp(ins_char, "SUB") == 0) return iSUB;
    if (strcmp(ins_char, "MOV_IN") == 0) return iMOV_IN;
    if (strcmp(ins_char, "MOV_OUT") == 0) return iMOV_OUT;
    if (strcmp(ins_char, "SUM") == 0) return iSUM;
    if (strcmp(ins_char, "JNZ") == 0) return iJNZ;
    if (strcmp(ins_char, "RESIZE") == 0) return iRESIZE;
    if (strcmp(ins_char, "COPY_STRING") == 0) return iCOPY_STRING;
    if (strcmp(ins_char, "WAIT") == 0) return iWAIT;
    if (strcmp(ins_char, "SIGNAL") == 0) return iSIGNAL;
    if (strcmp(ins_char, "IO_GEN_SLEEP") == 0) return iIO_GEN_SLEEP;
    if (strcmp(ins_char, "IO_STDIN_READ") == 0) return iIO_STDIN_READ;
    if (strcmp(ins_char, "IO_STDOUT_WRITE") == 0) return iIO_STDOUT_WRITE;
    if (strcmp(ins_char, "IO_FS_CREATE") == 0) return iIO_FS_CREATE;
    if (strcmp(ins_char, "IO_FS_DELETE") == 0) return iIO_FS_DELETE;
    if (strcmp(ins_char, "IO_FS_TRUNCATE") == 0) return iIO_FS_TRUNCATE;
    if (strcmp(ins_char, "IO_FS_WRITE") == 0) return iIO_FS_WRITE;
    if (strcmp(ins_char, "IO_FS_READ") == 0) return iIO_FS_READ;
    if (strcmp(ins_char, "EXIT") == 0) return iEXIT;

    fprintf(stderr, "Instruccion desconocida '%s'\n", ins_char);
    exit(EXIT_FAILURE);
}

t_list* dividir_cadena_en_tokens(char* linea){
    t_list* lista = list_create();
    char* cadena = strdup(linea);
    char* token = strtok(cadena, " ");

    while(token != NULL){
        list_add(lista, strdup(token));
        token = strtok(NULL," ");
    }

    free(cadena);
    free(linea);
    
    return lista;
}

void liberar_instruccion(t_instruccion* instruccion) {
    switch(instruccion->tipo){
        case iSET:
        case iSUM:
        case iSUB:
        case iJNZ:
        case iMOV_IN:
        case iMOV_OUT:
            free(instruccion->arg1);
            free(instruccion->arg2);
            break;
        case iRESIZE:
        case iCOPY_STRING:
        case iWAIT:
        case iSIGNAL:
            free(instruccion->arg1);
            break;
        case iIO_GEN_SLEEP:
            free(instruccion->interfaz);
            free(instruccion->arg1);
            break;
        case iIO_STDOUT_WRITE:
        case iIO_STDIN_READ:
            free(instruccion->interfaz);
            free(instruccion->arg1);
            free(instruccion->arg2);
            break;
        case iIO_FS_CREATE:
        case iIO_FS_DELETE:
            free(instruccion->interfaz);
            free(instruccion->archivo);
            break;
        case iIO_FS_TRUNCATE:
            free(instruccion->interfaz);
            free(instruccion->archivo);
            free(instruccion->arg1);
            break;
        case iIO_FS_WRITE:
        case iIO_FS_READ:
            free(instruccion->interfaz);
            free(instruccion->archivo);
            free(instruccion->arg1);
            free(instruccion->arg2);
            free(instruccion->arg3);
            break;
        case iEXIT:
            break; // No hay nada que liberar para la instrucción de salida
        default:
            break;
    }
    free(instruccion);
}

// Funciones necesarias para el ciclo de instrucciones

PCB* recibir_pcb(){
    op_code cod_op = recibir_operacion(socket_kernel_d);
    if(cod_op == ENVIO_PCB){
        t_buffer* buffer = recibir_buffer(socket_kernel_d);
        PCB* pcb_recibido = buffer_read_pcb(buffer);
        liberar_buffer(buffer);
        return pcb_recibido;
    }
    return NULL;
}

void enviar_pcb(op_code motivo){
    t_paquete* paquete = crear_paquete(motivo);
    agregar_PCB_a_paquete (paquete, pcb);
    enviar_paquete(paquete, socket_kernel_d);
    eliminar_paquete(paquete);
}

void enviar_PC_a_memoria(uint32_t pc){
    t_paquete* paquete = crear_paquete(ENVIO_DE_INSTRUCCIONES);
    agregar_int_a_paquete(paquete, pcb->PID);
    agregar_a_paquete(paquete, &pc, sizeof(uint32_t));
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}

char* obtener_instruccion_de_memoria(){
    op_code cod_op = recibir_operacion(socket_memoria);
    if (cod_op == ENVIO_DE_INSTRUCCIONES) { 
        t_buffer* buffer = recibir_buffer(socket_memoria);
        char* instruccion = buffer_read_string(buffer);
        liberar_buffer(buffer);
        return instruccion;
    } else { 
        log_error(log_cpu, "Error al recibir instrucción de la memoria");
        return NULL;
    }
}

//Ciclo de instrucciones

char* fetch(){
    log_info(log_cpu, "PID: %u - FETCH - Program Counter: %u", pcb->PID, pcb->registros.PC);

    enviar_PC_a_memoria(pcb->registros.PC);
    char* instruccionEncontrada = obtener_instruccion_de_memoria();

    pcb->registros.PC++; // Para contemplar el caso del SET PC

    return instruccionEncontrada;
}

t_instruccion* decode(char* instruccion_sin_decodificar){

    t_list* lista = dividir_cadena_en_tokens(instruccion_sin_decodificar);
    t_tipoInstruccion tipo_de_instruccion = string_a_tipo_instruccion(list_get(lista,0));
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    registrosCPU* argumento = malloc(sizeof(registrosCPU));
    registrosCPU* argumento2 = malloc(sizeof(registrosCPU));
    registrosCPU* argumento3 = malloc(sizeof(registrosCPU));

    switch(tipo_de_instruccion){
        case iSET:
            instruccion->tipo = iSET;
            *argumento = string_a_registro(list_get(lista,1));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->sizeArg2 = sizeof(int);
            *argumento2 = atoi(list_get(lista,2));
            instruccion->arg2 = argumento2;
            instruccion->sizeArg3 = 0;
            free(argumento3);
            break;
        case iSUM:
            instruccion->tipo = iSUM;
            *argumento = string_a_registro(list_get(lista,1));
            *argumento2 = string_a_registro(list_get(lista,2));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->arg2 = argumento2;
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,2)));
            instruccion->sizeArg3 = 0;
            free(argumento3);
            break;
        case iSUB:
            instruccion->tipo = iSUB;
            *argumento = string_a_registro(list_get(lista,1));
            *argumento2 = string_a_registro(list_get(lista,2));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->arg2 = argumento2;
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,2)));
            instruccion->sizeArg3 = 0;
            free(argumento3);
            break;
        case iJNZ:
            instruccion->tipo = iJNZ;
            *argumento = string_a_registro(list_get(lista,1));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->sizeArg2 = sizeof(int);
            *argumento2 = atoi(list_get(lista,2));
            instruccion->arg2 = argumento2;
            instruccion->sizeArg3 = 0;
            free(argumento3);
            break;
        case iIO_GEN_SLEEP:
            instruccion->tipo = iIO_GEN_SLEEP;
            *argumento = atoi(list_get(lista,2));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = sizeof(int);
            instruccion->sizeArg2 = 0;
            instruccion->sizeArg3 = 0;
            instruccion->interfaz = strdup(list_get(lista,1));
            free(argumento2);
            free(argumento3);
            break;
        case iRESIZE:
            instruccion->tipo = iRESIZE;
            *argumento = atoi(list_get(lista,1));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = sizeof(uint32_t);
            instruccion->sizeArg2 = 0;
            instruccion->sizeArg3 = 0;
            free(argumento2);
            free(argumento3);
            break;
        case iMOV_IN:
            instruccion->tipo = iMOV_IN;
            *argumento = string_a_registro(list_get(lista,1));
            *argumento2 = string_a_registro(list_get(lista,2));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->arg2 = argumento2;
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,2)));
            instruccion->sizeArg3 = 0;
            free(argumento3);
            break;
        case iMOV_OUT:
            instruccion->tipo = iMOV_OUT;
            *argumento = string_a_registro(list_get(lista,1));
            *argumento2 = string_a_registro(list_get(lista,2));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->arg2 = argumento2;
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,2)));
            instruccion->sizeArg3 = 0;
            free(argumento3);
            break;
        case iCOPY_STRING:
            instruccion->tipo = iCOPY_STRING;
            *argumento = atoi(list_get(lista,1));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = sizeof(uint32_t);
            instruccion->sizeArg2 = 0;
            instruccion->sizeArg3 = 0;
            free(argumento2);
            free(argumento3);
            break;
        case iIO_STDIN_READ:
            instruccion->tipo = iIO_STDIN_READ;
            instruccion->interfaz = strdup(list_get(lista, 1));
            *argumento = string_a_registro(list_get(lista, 2));
            *argumento2 = string_a_registro(list_get(lista, 3));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,2)));
            instruccion->arg2 = argumento2;
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,3)));
            instruccion->sizeArg3 = 0;
            free(argumento3);
            break;
        case iIO_STDOUT_WRITE:
            instruccion->tipo = iIO_STDOUT_WRITE;
            instruccion->interfaz = strdup(list_get(lista,1));
            *argumento = string_a_registro(list_get(lista,2));
            *argumento2 = string_a_registro(list_get(lista,3));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,2)));
            instruccion->arg2 = argumento2;
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,3)));
            instruccion->sizeArg3 = 0;
            free(argumento3);
            break;
        case iWAIT:
            instruccion->tipo = iWAIT;
            instruccion->arg1 = strdup(list_get(lista,1));
            instruccion->sizeArg1 = strlen(list_get(lista,1))+1;
            instruccion->sizeArg2 = 0;
            instruccion->sizeArg3 = 0;
            free(argumento);
            free(argumento2);
            free(argumento3);
            break;
        case iSIGNAL:
            instruccion->tipo = iSIGNAL;
            instruccion->arg1 = strdup(list_get(lista,1));
            instruccion->sizeArg1 = strlen(list_get(lista,1))+1;
            instruccion->sizeArg2 = 0;
            instruccion->sizeArg3 = 0;
            free(argumento);
            free(argumento2);
            free(argumento3);
            break;
        case iIO_FS_CREATE:
            instruccion->tipo = iIO_FS_CREATE;
            instruccion->interfaz = strdup(list_get(lista,1));
            instruccion->archivo = strdup(list_get(lista,2));
            instruccion->sizeArg1 = 0;
            instruccion->sizeArg2 = 0;
            instruccion->sizeArg3 = 0;
            free(argumento);
            free(argumento2);
            free(argumento3);
        break;
        case iIO_FS_DELETE:
            instruccion->tipo = iIO_FS_DELETE;
            instruccion->interfaz = strdup(list_get(lista,1));
            instruccion->archivo = strdup(list_get(lista,2));
            instruccion->sizeArg1 = 0;
            instruccion->sizeArg2 = 0;
            instruccion->sizeArg3 = 0;
            free(argumento);
            free(argumento2);
            free(argumento3);
        break;
        case iIO_FS_TRUNCATE:
            instruccion->tipo = iIO_FS_TRUNCATE;
            instruccion->interfaz = strdup(list_get(lista,1));
            instruccion->archivo = strdup(list_get(lista,2));
            *argumento = string_a_registro(list_get(lista,3));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista, 3)));
            instruccion->sizeArg3 = 0;
            instruccion->sizeArg2 = 0;
            free(argumento2);
            free(argumento3);
        break;
        case iIO_FS_WRITE:
            instruccion->tipo = iIO_FS_WRITE;
            instruccion->interfaz = strdup(list_get(lista,1));
            instruccion->archivo = strdup(list_get(lista,2));
            *argumento = string_a_registro(list_get(lista, 3));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,3)));
            *argumento2 = string_a_registro(list_get(lista,4));
            instruccion->arg2 = argumento2;
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,4)));
            *argumento3 = string_a_registro(list_get(lista,5));
            instruccion->arg3 = argumento3;
            instruccion->sizeArg3 = tamanioRegistro(string_a_registro(list_get(lista,5)));
        break;
        case iIO_FS_READ:
            instruccion->tipo = iIO_FS_READ;
            instruccion->interfaz = strdup(list_get(lista, 1));
            instruccion->archivo = strdup(list_get(lista, 2));
            *argumento = string_a_registro(list_get(lista, 3));
            instruccion->arg1 = argumento;
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,3)));
            *argumento2 = string_a_registro(list_get(lista,4));
            instruccion->arg2 = argumento2;
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,4)));
            *argumento3 = string_a_registro(list_get(lista,5));
            instruccion->arg3 = argumento3;
            instruccion->sizeArg3 = tamanioRegistro(string_a_registro(list_get(lista,5)));
        break;
        case iEXIT:
            instruccion->tipo = iEXIT;
            free(argumento);
            free(argumento2);
            free(argumento3);
            break;
        default:
            break;
    }

    list_destroy_and_destroy_elements(lista, free);

    return instruccion;
}

void execute(t_instruccion* instruccion){
    switch (instruccion->tipo)
    {
    case iSET:
        log_info(log_cpu, "PID: %u - Ejecutando: SET - Parametro 1: %s, Parametro 2: %u", pcb->PID, registro_a_string(*(registrosCPU*)instruccion->arg1), *(int*)instruccion->arg2);
        SET(*(registrosCPU *)instruccion->arg1, *(int *)instruccion->arg2);
        break;
    case iMOV_IN:
        log_info(log_cpu, "PID: %u - Ejecutando: MOV_IN - Parametro 1: %s, Parametro 2: %s", pcb->PID, registro_a_string(*(registrosCPU*)instruccion->arg1), registro_a_string(*(registrosCPU*)instruccion->arg2));
        MOV_IN(*(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);
        break;
    case iMOV_OUT:
        log_info(log_cpu, "PID: %u - Ejecutando: MOV_OUT - Parametro 1: %s, Parametro 2: %s", pcb->PID, registro_a_string(*(registrosCPU*)instruccion->arg1), registro_a_string(*(registrosCPU*)instruccion->arg2));
        MOV_OUT(*(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);
        break;
    case iSUM:
        log_info(log_cpu, "PID: %u - Ejecutando: SUM - Parametro 1: %s, Parametro 2: %s", pcb->PID, registro_a_string(*(registrosCPU*)instruccion->arg1), registro_a_string(*(registrosCPU*)instruccion->arg2));
        SUM(*(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);
        break;
    case iSUB:
        log_info(log_cpu, "PID: %u - Ejecutando: SUB - Parametro 1: %s, Parametro 2: %s", pcb->PID, registro_a_string(*(registrosCPU*)instruccion->arg1), registro_a_string(*(registrosCPU*)instruccion->arg2));
        SUB(*(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);
        break;
    case iJNZ:
        log_info(log_cpu, "PID: %u - Ejecutando: JNZ - Parametro 1: %s, Parametro 2: %u", pcb->PID, registro_a_string(*(registrosCPU*)instruccion->arg1), *(int*)instruccion->arg2);
        JNZ(*(registrosCPU *)instruccion->arg1, *(int *)instruccion->arg2);
        break;
    case iRESIZE:
        log_info(log_cpu, "PID: %u - Ejecutando: RESIZE - Parametro 1: %u", pcb->PID, *(int*)instruccion->arg1);
        RESIZE(*(int *)instruccion->arg1);
        break;
    case iCOPY_STRING:
        log_info(log_cpu, "PID: %u - Ejecutando: COPY_STRING - Parametro 1: %u", pcb->PID, *(int*)instruccion->arg1);
        COPY_STRING(*(int *)instruccion->arg1);
        break;
    case iWAIT:
        log_info(log_cpu, "PID: %u - Ejecutando: WAIT - Parametro 1: %s", pcb->PID, (char*)instruccion->arg1);
        WAIT((char*)instruccion->arg1);
        break;
    case iSIGNAL:
        log_info(log_cpu, "PID: %u - Ejecutando: SIGNAL - Parametro 1: %s", pcb->PID, (char*)instruccion->arg1);
        SIGNAL((char*)instruccion->arg1);
        break;
    case iIO_GEN_SLEEP:
        log_info(log_cpu, "PID: %u - Ejecutando: IO_GEN_SLEEP - Parametro 1: %s, Parametro 2: %u", pcb->PID, (char*)instruccion->interfaz, *(int*)instruccion->arg1);
        IO_GEN_SLEEP((char*)instruccion->interfaz, *(int *)instruccion->arg1);   
        break;
    case iIO_STDIN_READ:
        log_info(log_cpu, "PID: %u - Ejecutando: IO_STDIN_READ - Parametro 1: %s, Parametro 2: %s, Parametro 3: %s", pcb->PID, (char*)instruccion->interfaz, registro_a_string(*(registrosCPU*)instruccion->arg1), registro_a_string(*(registrosCPU*)instruccion->arg2));
        IO_STDIN_READ((char*)instruccion->interfaz, *(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);   
        break;
    case iIO_STDOUT_WRITE:
        log_info(log_cpu, "PID: %u - Ejecutando: IO_STDOUT_WRITE - Parametro 1: %p, Parametro 2: %s, Parametro 3: %s", pcb->PID, (char*)instruccion->interfaz, registro_a_string(*(registrosCPU*)instruccion->arg1), registro_a_string(*(registrosCPU*)instruccion->arg2));
        IO_STDOUT_WRITE((char*)instruccion->interfaz, *(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2);    
        break;
    case iIO_FS_CREATE:
        log_info(log_cpu, "PID: %u - Ejecutando: IO_FS_CREATE - Parametro 1: %s, Parametro 2: %s", pcb->PID, (char*)instruccion->interfaz, (char*)instruccion->archivo);
        IO_FS_CREATE((char*)instruccion->interfaz, (char*)instruccion->archivo);
        break;
    case iIO_FS_DELETE:
        log_info(log_cpu, "PID: %u - Ejecutando: IO_FS_DELETE - Parametro 1: %s, Parametro 2: %s", pcb->PID, (char*)instruccion->interfaz, (char*)instruccion->archivo);
        IO_FS_DELETE((char*)instruccion->interfaz, (char*)instruccion->archivo);
        break;
    case iIO_FS_TRUNCATE:
        log_info(log_cpu, "PID: %u - Ejecutando: IO_FS_TRUNCATE - Parametro 1: %s, Parametro 2: %s, Parametro 3: %s", pcb->PID, (char*)instruccion->interfaz, (char*)instruccion->archivo, registro_a_string(*(registrosCPU *)instruccion->arg1));
        IO_FS_TRUNCATE((char*)instruccion->interfaz, (char*)instruccion->archivo, *(registrosCPU *)instruccion->arg1);
        break;
    case iIO_FS_WRITE:
        log_info(log_cpu, "PID: %u - Ejecutando: IO_FS_WRITE - Parametro 1: %s, Parametro 2: %s, Parametro 3: %s, Parametro 4: %s, Parametro 5: %s", pcb->PID, (char*)instruccion->interfaz, (char*)instruccion->archivo, registro_a_string(*(registrosCPU *)instruccion->arg1), registro_a_string(*(registrosCPU *)instruccion->arg2), registro_a_string(*(registrosCPU *)instruccion->arg3));
        IO_FS_WRITE((char*)instruccion->interfaz,( char*)instruccion->archivo, *(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2, *(registrosCPU *)instruccion->arg3);
        break;
    case iIO_FS_READ:
        log_info(log_cpu, "PID: %u - Ejecutando: IO_FS_READ - Parametro 1: %s, Parametro 2: %s, Parametro 3: %s, Parametro 4: %s, Parametro 5: %s", pcb->PID, (char*)instruccion->interfaz, (char*)instruccion->archivo, registro_a_string(*(registrosCPU *)instruccion->arg1), registro_a_string(*(registrosCPU *)instruccion->arg2), registro_a_string(*(registrosCPU *)instruccion->arg3));
        IO_FS_READ((char*)instruccion->interfaz, (char*)instruccion->archivo, *(registrosCPU *)instruccion->arg1, *(registrosCPU *)instruccion->arg2, *(registrosCPU *)instruccion->arg3);
        break;
    case iEXIT:
        log_info(log_cpu, "PID: %u - Ejecutando: EXIT - No tiene parámetros", pcb->PID);
        EXIT();
        break;
    default:
        log_error(log_cpu, "La instruccion es inválida");
        break;
    }
}

int check_interrupt() {
    pthread_mutex_lock(&mutexInterrupt);
    if (hay_interrupcion) {
        hay_interrupcion = 0;
        pthread_mutex_unlock(&mutexInterrupt);
        return 1;
    } else {
        pthread_mutex_unlock(&mutexInterrupt);
        return 0;
    }
}

void realizar_ciclo_de_instruccion(){
    int terminar = 0;
    while (!terminar) {
        char* instruccion_a_decodificar = fetch();

        t_instruccion* instruccion_a_ejecutar = decode(instruccion_a_decodificar);
        
        execute(instruccion_a_ejecutar);
        
        t_tipoInstruccion tipo_de_instr = instruccion_a_ejecutar->tipo;

        switch (tipo_de_instr) {
        case iRESIZE:
            op_code cod_op = recibir_operacion(socket_memoria);
            if(cod_op == OUT_OF_MEMORY){
                enviar_pcb(cod_op);
                terminar = 1;
            }
            break;
        case iIO_GEN_SLEEP:
        case iIO_STDIN_READ:
        case iIO_STDOUT_WRITE:
        case iIO_FS_CREATE:
        case iIO_FS_DELETE:
        case iIO_FS_TRUNCATE:
        case iIO_FS_READ:
        case iIO_FS_WRITE:
        case iEXIT:
            terminar = 1;  
            break;
        default:
            break;
        }

        liberar_instruccion(instruccion_a_ejecutar);
        
        if (check_interrupt()) {
            enviar_pcb(cod_op_int);
            terminar = 1;
        }
    }
}


