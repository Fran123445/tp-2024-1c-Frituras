#include "registroCpu.h"

void* obtenerRegistro(registrosCPU);
size_t tamanioRegistro(registrosCPU);

void SET(registrosCPU registro, int valor){
    void *reg_a_setear= obtenerRegistro(registro);

    if (reg_a_setear == NULL) { // Tengo entendido que no van a haber errores en lo que nos van a pasar para probar, podríamos evitar esto. 
        printf("Error: Registro inválido.\n"); 
        return;
    }

    size_t tam_reg = tamanoRegistro(registro);


    switch (tam_reg) {
        case sizeof(uint8_t):
            *(uint8_t *)reg_a_setear = (uint8_t)valor; 
            break;
        case sizeof(uint32_t):
            *(uint32_t *)reg_a_setear = (uint32_t)valor;
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


    switch (tam_destino) {  // Seguramente haya alguna manera más eficiente de hacerlo, habría que buscar
        case sizeof(uint8_t):
            switch(tam_origen){
                case sizeof(uint8_t):
                    *(uint8_t *)reg_destino += *(uint8_t *)reg_origen; break;
                case sizeof(uint32_t):
                    *(uint8_t *)reg_destino += *(uint32_t *)reg_origen; break; // Se puede hacer esto?
            }
            break;
        case sizeof(uint32_t):
            switch(tam_origen){
                case sizeof(uint8_t):
                    *(uint32_t *)reg_destino += *(uint8_t *)reg_origen; break;
                case sizeof(uint32_t):
                    *(uint32_t *)reg_destino += *(uint32_t *)reg_origen; break;
            }
            break;
    }
}

void* obtenerRegistro(registrosCPU registro) {
    switch (registro) {  // Podríamos cambiarlo a una lista si querés evitar usar switch, seguramente sea más eficiente en cuanto a recursos y deberíamos hacerlo.
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
        case PC: return &miCPU.PC;

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