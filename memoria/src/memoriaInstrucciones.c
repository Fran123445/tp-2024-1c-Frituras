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
void* obtener_instruccion(int socket_kernel, int pc, int pid){
    t_proceso* proceso = hallar_proceso(pid);
    t_list* lista_instrucciones = proceso->instrucciones;
    char* instruccion_char = list_get(lista_instrucciones, pc);
    free(lista_instrucciones);
    t_list* dividir_cadena_en_tokens(instruccion_char);
    t_tipoInstruccion tipo_de_instruccion = malloc(sizeof(t_tipoInstruccion));
    tipo_de_instruccion = obtener_tipo_instruccion(list_get(dividir_cadena_en_tokens(instruccion_char),0));
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    switch(tipo_de_instruccion){
        case iSET:
            instruccion = crear_instruccion(tipo_de_instruccion, 
                            tamanioRegistro(string_a_registro(list_get(dividir_cadena_en_tokens(instruccion_char),1))),
                            string_a_registro(list_get(dividir_cadena_en_tokens(instruccion_char),1)),
                            sizeof(int), atoi(list_get(dividir_cadena_en_tokens(instruccion_char),2)),
                            NULL, NULL, NULL, NULL);
                            
            break;
        case iMOV_IN:
            // a implementar 
            break;
        case iMOV_OUT:
            //crear_instruccion();
            break;
        case iSUM:
        instruccion = crear_instruccion(tipo_de_instruccion,
                    tamanioRegistro(string_a_registro(list_get(dividir_cadena_en_tokens(instruccion_char),1))),
                    string_a_registro(list_get(dividir_cadena_en_tokens(instruccion_char),1)),
                    tamanioRegistro(string_a_registro(list_get(dividir_cadena_en_tokens(instruccion_char),2))), 
                    string_a_registro(list_get(dividir_cadena_en_tokens(instruccion_char),2)),
                    NULL, NULL, NULL, NULL);
        case iSUB:
        instruccion = crear_instruccion(tipo_de_instruccion,
                    tamanioRegistro(string_a_registro(list_get(dividir_cadena_en_tokens(instruccion_char),1))),
                    string_a_registro(list_get(dividir_cadena_en_tokens(instruccion_char),1)),
                    sizeof(int), list_get(dividir_cadena_en_tokens(instruccion_char),2),
                    NULL, NULL, NULL, NULL);
                    break;
        case iJNZ:
        instruccion = crear_instruccion(tipo_de_instruccion,
                    tamanioRegistro(string_a_registro(list_get(dividir_cadena_en_tokens(instruccion_char),1))),
                    string_a_registro(list_get(dividir_cadena_en_tokens(instruccion_char),1)),
                    sizeof(int), atoi(list_get(dividir_cadena_en_tokens(instruccion_char),2)),
                    NULL, NULL, NULL, NULL);
                    break;
        case iIO_GEN_SLEEP:
        instruccion = crear_instruccion(tipo_de_instruccion,
                    sizeof(int), atoi(list_get(dividir_cadena_en_tokens(instruccion_char),2)),
                    NULL, NULL, NULL, NULL,
                    list_get(dividir_cadena_en_tokens(instruccion_char,1)),
                      NULL);
                    break;
        
    }
    
    }


t_instruccion* crear_instruccion(t_tipoInstruccion tipo_instruccion, int sizeArg, void* arg1, int sizeArg2, void* arg2, int sizeArg3, void* arg3, char* interfaz, char* archivo){
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    instruccion->tipo = tipo_instruccion;
    instruccion->sizeArg1 = sizeArg;
    instruccion->arg1 = arg1;
    instruccion->sizeArg2 = sizeArg2;
    instruccion->arg2 = arg2;
    instruccion->sizeArg3 = sizeArg3;
    instruccion-> arg3 = arg3;
    instruccion->interfaz = interfaz;
    instruccion->archivo = archivo;
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

