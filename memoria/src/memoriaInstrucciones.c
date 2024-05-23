#include <utils/serializacion.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <utils/pcb.h>
#include "estructuras.h"
#include "memoriaInstrucciones.h"
t_tipoInstruccion obtener_tipo_instruccion (const* char ins_char){
    if (strcmp(ins_char, "SET") == 0) return SET;
    if (strcmp(ins_char, "MOV_IN") == 0) return MOV_IN;
    if (strcmp(ins_char, "MOV_OUT") == 0) return MOV_OUT;
    if (strcmp(ins_char, "SUM") == 0) return SUM;
    if (strcmp(ins_char, "JNZ") == 0) return JNZ;
    if (strcmp(ins_char, "RESIZE") == 0) return RESIZE;
    if (strcmp(ins_char, "COPY_STRING") == 0) return COPY_STRING;
    if (strcmp(ins_char, "WAIT") == 0) return WAIT;
    if (strcmp(ins_char, "SIGNAL") == 0) return SIGNAL;
    if (strcmp(ins_char, "IO_GEN_SLEEP")) return IO_GEN_SLEEP;
    if (strcmp(ins_char, "IO_STDIN_READ") == 0) return IO_STDIN_READ;
    if (strcmp(ins_char, "IO_STDOUT_WRITE") == 0) return IO_STDOUT_WRITE;
    if (strcmp(ins_char, "IO_FS_CREATE") == 0) return IO_FS_CREATE;
    if (strcmp(ins_char, "IO_FS_DELETE") == 0) return IO_FS_DELETE;
    if (strcmp(ins_char, "IO_FS_TRUNCATE") == 0) return IO_FS_TRUNCATE;
    if (strcmp(ins_char, "IO_FS_WRITE") == 0) return IO_FS_WRITE;
    if (strcmp(ins_char, "IO_FS_READ") == 0) return IO_FS_READ;
    if (strcmp(ins_char, "EXIT") == 0) return EXIT;

    fprintf(stderr, "Instruccion desconocida '%s'\n", ins_char);
    exit(EXIT_FAILURE);
}

t_instruccion* volver_char_a_instruccion(char*linea){
    char* espacio = strchr(linea, ' ');
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    t_tipoInstruccion tipo_ins = obtener_tipo_instruccion(linea);
    switch(tipo_ins){
        case EXIT:
            instruccion->tipo= EXIT;
            instruccion->sizeArg1 = NULL;
            instruccion->arg1 = NULL;
            instruccion->sizeArg2 = NULL;
            instruccion->arg2 = NULL;
            instruccion->sizeArg3 = NULL;
            instruccion->arg3 = NULL;
            instruccion->interfaz = NULL;
            instruccion->archivo = NULL;
            break;
        case SET:
            instruccion->tipo = SET;
            instruccion->sizeArg1 = tamanioRegistro(registro);
            instruccion->arg1 = registros_cpu;
            instruccion->sizeArg2 = sizeof(int);
            instruccion->arg2 = int;
            break;
        case MOV_IN:
        break;
        case MOV_OUT:
        break;
        case SUM:
        break;
        case SUB:
        break;
        case JNZ:
        break;
        case RESIZE:
        break;
        case COPY_STRING:
        break;
        case WAIT:
        break;
        case SIGNAL:
        break;
        case IO_GEN_SLEEP:
        break;
        case IO_STDIN_READ:
        break;
        case IO_STDOUT_WRITE:
        break;
        case IO_FS_CREATE:
        break;
        case IO_FS_DELETE:
        break;
        case IO_FS_TRUNCATE:
        break;
        case IO_FS_READ:
        break;
        case IO_FS_READ:
        break;

    }

    }

