#include <utils/serializacion.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <utils/pcb.h>
#include "estructuras.h"
#include "memoriaInstrucciones.h"
#include "main.h"
#include "buscarprocesos.h"

t_tipoInstruccion obtener_tipo_instruccion (const* char ins_char){
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

t_instruccion* volver_char_a_instruccion(char*linea){
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    t_tipoInstruccion tipo_ins = obtener_tipo_instruccion(linea);
    char* espacio = strchr(linea, ' ');
    registrosCPU registro1;
    registrosCPU registro2;
    registrosCPU registro3;
    switch(tipo_ins){
        case iEXIT:
            instruccion->tipo= iEXIT;
            instruccion->sizeArg1 = NULL;
            instruccion->arg1 = NULL;
            instruccion->sizeArg2 = NULL;
            instruccion->arg2 = NULL;
            instruccion->sizeArg3 = NULL;
            instruccion->arg3 = NULL;
            instruccion->interfaz = NULL;
            instruccion->archivo = NULL;
            break;
        case iSET:
            instruccion->tipo = iSET;
            instruccion->sizeArg1 = tamanioRegistro(registro);
            instruccion->arg1 = registro1;
            instruccion->sizeArg2 = sizeof(int);
            instruccion->arg2 = int;
            break;
        case iMOV_IN:
            instruccion->tipo = MOV_IN;
            instruccion->sizeArg1 =;
            instruccion->arg1 =;
            instruccion->sizeArg2 = ;
            instruccion->arg2 =;
            instruccion->sizeArg3 = NULL;
            instruccion->
        break;
        case iMOV_OUT:
        break;
        case iSUM:
        break;
        case iSUB:
        break;
        case iJNZ:
        break;
        case iRESIZE:
        break;
        case iCOPY_STRING:
        break;
        case iWAIT:
        break;
        case iSIGNAL:
        break;
        case iIO_GEN_SLEEP:
        break;
        case iIO_STDIN_READ:
        break;
        case iIO_STDOUT_WRITE:
        break;
        case iIO_FS_CREATE:
        break;
        case iIO_FS_DELETE:
        break;
        case iIO_FS_TRUNCATE:
        break;
        case iIO_FS_READ:
        break;
        case iIO_FS_READ:
        break;

    }

    }

void* obtener_instruccion(int socket_kernel, int pc, int pid){
    list_find(lista_de_procesos_con_ins, ); //
    char* instruccion_char = list_get(lista_con_ins, pc);
    free(lista_con_ins);
    t_list* dividir_cadena_en_tokens(instruccion_char);
    
    }

void* obtener_argumentos(const char* linea){
    t_tipoInstruccion tipo_ins = obtener_tipo_instruccion (linea);
    
    switch(){

    }

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