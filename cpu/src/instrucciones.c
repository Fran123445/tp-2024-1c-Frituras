#include "registroCpu.h"

void* obtenerRegistro(registrosCPU);
size_t tamanioRegistro(registrosCPU);

void SET(registrosCPU registro, uint32_t valor){
    void *reg_a_setear= obtenerRegistro(registro);

    if (reg_a_setear == NULL) {
        printf("Error: Registro inválido.\n");
        return;
    }

    size_t tam_reg = tamanoRegistro(registro);


    switch (tam_reg) {
        case sizeof(uint8_t):
            *(uint8_t *)reg_a_setear = (uint8_t)valor; // esto faltaría chequearlo, pero pedir el tipo de dato más grande directamente y achicarlo de ser necesario no suena tan mal.
            break;
        case sizeof(uint32_t):
            *(uint32_t *)reg_a_setear = valor;
            break;
    }
    //*(uint8_t *)reg_a_setear se utiliza para convertir el puntero genérico void *reg_a_setear a un puntero específico de tipo uint8_t *, lo cual es válido porque reg_a_setear apunta a un registro de 8 bits (AX, BX, CX, DX). Después de la conversión, la expresión dereferencia el puntero uint8_t * para obtener el valor almacenado en la dirección de memoria a la que apunta reg_a_setear.
}

void SUM(registrosCPU registroDestino, registrosCPU registroOrigen){
    void *reg_destino = obtenerRegistro(registroDestino);
    void *reg_origen = obtenerRegistro(registroOrigen);

    if (reg_destino == NULL || reg_origen == NULL) {
        printf("Error: Uno o ambos registros no válidos.\n");
        return;
    }

    size_t tam_destino = tamanoRegistro(registroDestino);
    size_t tam_origen = tamanoRegistro(registroOrigen);

    if(tam_destino != tam_origen){
        printf("Error: Los registros no son del mismo tamaño de bits.\n");
        return;  
    }

    switch (tam_destino) {
        case sizeof(uint8_t):
            *(uint8_t *)reg_destino += *(uint8_t *)reg_origen;
            break;
        case sizeof(uint32_t):
            *(uint32_t *)reg_destino += *(uint32_t *)reg_origen;
            break;
    }
}

void* obtenerRegistro(registrosCPU registro) {
    switch (registro) {
        case PC: return &miCPU.PC;
        case AX: return &miCPU.AX;
        case BX: return &miCPU.BX;
        case CX: return &miCPU.CX;
        case DX: return &miCPU.DX;
        case EAX: return &miCPU.EAX;
        case EBX: return &miCPU.EBX;
        case ECX: return &miCPU.ECX;
        case EDX: return &miCPU.EDX;
        case SI: return &miCPU.SI;
        case DI: return &miCPU.DI;

        default:
            return NULL;
    }
}

size_t tamanioRegistro(registrosCPU registro) {
    if (registro >= AX && registro <= DX) {
        return sizeof(uint8_t);
    } else {
        return sizeof(uint32_t);
    }
    // En las funciones que invocan a esto, siempre antes se verifica que el registro exista, así que no hace falta que lo verifique acá también.
}