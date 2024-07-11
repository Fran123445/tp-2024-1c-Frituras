#ifndef PCB_H
#define PCB_H

#include <stdlib.h>
#include <stdint.h> // Incluye esta línea si utilizas tipos de datos como uint8_t o uint32_t
#include <stdio.h>
#include <commons/string.h>

// Declaraciones de tipos de datos
typedef enum {
    AX,
    BX,
    CX,
    DX,
    EAX,
    EBX,
    ECX,
    EDX,
    SI,
    DI,
    PC
} registrosCPU;

typedef struct {
    // Definición de los registros de la CPU
    uint8_t AX, BX, CX, DX;
    uint32_t EAX, EBX, ECX, EDX;
    uint32_t SI, DI, PC;
} registros_cpu;

typedef enum {
    ESTADO_NEW,
    ESTADO_READY,
    ESTADO_BLOCKED,
    ESTADO_EXEC,
    ESTADO_EXIT
}estado_proceso;

typedef struct {
    int PID;
    int quantum;
    estado_proceso estado;
    registros_cpu registros;
    char** recursosAsignados;
}PCB;

//Tipos de instrucciones
typedef enum{
    iSET,
    iMOV_IN,
    iMOV_OUT,
    iSUM,
    iSUB,
    iJNZ,
    iRESIZE,
    iCOPY_STRING ,
    iWAIT, 
    iSIGNAL,
    iIO_GEN_SLEEP,
    iIO_STDIN_READ,
    iIO_STDOUT_WRITE,
    iIO_FS_CREATE,
    iIO_FS_DELETE,
    iIO_FS_TRUNCATE,
    iIO_FS_WRITE,
    iIO_FS_READ,
    iEXIT
}t_tipoInstruccion;

typedef struct {
    char* nombre;
    int unidades_trabajo;
}t_interfaz_generica;

typedef struct {
    t_tipoInstruccion tipo;
    int sizeArg1;
    void* arg1;
    int sizeArg2;    
    void* arg2;
    int sizeArg3;
    void* arg3;                        
    char* interfaz;       
    char* archivo;       
}t_instruccion;

size_t tamanioRegistro(registrosCPU registro);
void liberar_pcb(PCB* pcb);
#endif /* PCB_H */
