#ifndef PCB_H
#define PCB_H

#include <stdlib.h>
#include <stdint.h> // Incluye esta línea si utilizas tipos de datos como uint8_t o uint32_t
#include <stdio.h>

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
    uint32_t programCounter;
    int quantum;
    estado_proceso estado;
    registros_cpu registros;
}PCB;

//Tipos de instrucciones
typedef enum{
    SET,
    MOV_IN,
    MOV_OUT,
    SUM,
    SUB,
    JNZ,
    RESIZE,
    COPY_STRING ,
    WAIT, 
    SIGNAL,
    IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
    EXIT
}t_tipoInstruccion;

//Estructura de una instruccion: tiene el tipo, 3 argumentos void porque pueden ser numeros y/o registros(ejemplo set ax 10 -- sub ax cx -- resize 128 -- io_gen_sleep int3 4)
//interfaz es un char* para poder pasarle que interfaz es, y archivo va a pasar un nombre del archivo.
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

typedef struct {
    PCB* proceso;
    t_instruccion* instruccion;
}t_dispatch;

#endif /* PCB_H */
