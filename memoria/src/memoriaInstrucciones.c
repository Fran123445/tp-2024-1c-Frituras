#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <utils/pcb.h>
#include<unistd.h>
#include<netdb.h>
#include<pthread.h>
#include <semaphore.h>
#include "estructuras.h"
#include "memoriaInstrucciones.h"

pthread_mutex_t mutex_lista_de_procesos_con_ins = PTHREAD_MUTEX_INITIALIZER;

t_tipoInstruccion obtener_tipo_instruccion (char* ins_char){
    if (strcmp(ins_char, "SET") == 0) return iSET;
    if (strcmp(ins_char, "MOV_IN") == 0) return iMOV_IN;
    if (strcmp(ins_char, "MOV_OUT") == 0) return iMOV_OUT;
    if (strcmp(ins_char, "SUM") == 0) return iSUM;
    if (strcmp(ins_char, "JNZ") == 0) return iJNZ;
    if (strcmp(ins_char, "RESIZE") == 0) return iRESIZE;
    if (strcmp(ins_char, "COPY_STRING") == 0) return iCOPY_STRING;
    if (strcmp(ins_char, "WAIT") == 0) return iWAIT;
    if (strcmp(ins_char, "SIGNAL") == 0) return iSIGNAL;
    if (strcmp(ins_char, "IO_GEN_SLEEP")) return iIO_GEN_SLEEP;
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
t_instruccion* obtener_instruccion(int socket_kernel, int pc, int pid){
    t_proceso* proceso = hallar_proceso(pid);
    t_list* lista_instrucciones = proceso->instrucciones;
    char* instruccion_char = list_get(lista_instrucciones, pc);
    free(lista_instrucciones);
    t_list* lista = dividir_cadena_en_tokens(instruccion_char);
    t_tipoInstruccion tipo_de_instruccion = obtener_tipo_instruccion(list_get(lista,0));
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    int valor;
    int*valor_ptr = malloc(sizeof(int));
    switch(tipo_de_instruccion){
        case iSET:
            valor = atoi(list_get(lista,2));
            if(valor_ptr == NULL){
                perror("error de memoria");
                exit(EXIT_FAILURE);
            }
            *valor_ptr = valor;
            instruccion->tipo = iSET;
            instruccion->arg1 = (void*) string_a_registro(list_get(lista,1));
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->sizeArg2 = sizeof(int);
            instruccion->arg2 = valor_ptr;
            instruccion->sizeArg3 = 0;
            break;
        case iSUM:
            instruccion->tipo = iSUM;
            instruccion->arg1 = (void*) string_a_registro(list_get(lista,1));
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->arg2 = (void*) string_a_registro(list_get(lista,2));
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->sizeArg3 = 0;
            break;
        case iSUB:
            instruccion->tipo = iSUB;
            instruccion->arg1 = (void*) string_a_registro(list_get(lista,1));
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->arg2 = (void*) string_a_registro(list_get(lista,2));
            instruccion->sizeArg2 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->sizeArg3 = 0;
            break;
        case iJNZ:
            valor = atoi(list_get(lista,2));
            if(valor_ptr == NULL){
                perror("error de memoria");
                exit(EXIT_FAILURE);
            }
            *valor_ptr = valor;
            instruccion->tipo = iJNZ;
            instruccion->arg1 = (void*) string_a_registro(list_get(lista,1));
            instruccion->sizeArg1 = tamanioRegistro(string_a_registro(list_get(lista,1)));
            instruccion->sizeArg2 = sizeof(int);
            instruccion->arg2 = valor_ptr;
            instruccion->sizeArg3 = 0;
            break;
        case iIO_GEN_SLEEP:
            valor = atoi(list_get(lista,2));
            if(valor_ptr == NULL){
                perror("error de memoria");
                exit(EXIT_FAILURE);
            }
            *valor_ptr = valor;
            instruccion->tipo = iIO_GEN_SLEEP;
            instruccion->arg1 = valor_ptr;
            instruccion->sizeArg1 = sizeof(int);
            instruccion->sizeArg2 = 0;
            instruccion->sizeArg3 = 0;
            instruccion->interfaz = list_get(lista,1);
            break;
        default :
        break;

    }
         return instruccion;
}

t_list* dividir_cadena_en_tokens(const char* linea){
    t_list* lista = list_create();
    char* cadena = strdup(linea);
    char* token = strtok(cadena, " ");

    while(token != NULL){
        list_add(lista,token);
        token = strtok(NULL," ");
    }
    free(cadena);
    return lista;
}

t_proceso* hallar_proceso(int PID){
    bool _mismoPID(t_proceso* proceso){
        return (proceso->pid == PID);
    };

    pthread_mutex_lock(&mutex_lista_de_procesos_con_ins);
    t_proceso* encontrado = list_find(lista_de_procesos_con_ins, (void *)_mismoPID);
    pthread_mutex_unlock(&mutex_lista_de_procesos_con_ins);

    return encontrado;
}
