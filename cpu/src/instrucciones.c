#include "instrucciones.h"

registros_cpu miCPU;

void* obtenerRegistro(registrosCPU registro) {
    void* lista_de_registros[11] = {
        &miCPU.AX, &miCPU.BX, &miCPU.CX, &miCPU.DX,
        &miCPU.EAX, &miCPU.EBX, &miCPU.ECX, &miCPU.EDX,
        &miCPU.SI, &miCPU.DI, &miCPU.PC
    };

    if(registro >= 0 && registro < 11){
        return(lista_de_registros[registro]);
    }
    else{
        return NULL;
    }
    }

void SET(registrosCPU registro, int valor){
    void *reg_a_setear= obtenerRegistro(registro);

    if (reg_a_setear == NULL) { 
        printf("Error: Registro inválido.\n"); 
        return;
    }

    size_t tam_reg = tamanioRegistro(registro);


    switch (tam_reg) {
        case sizeof(uint8_t):
            *(uint8_t *)reg_a_setear = (uint8_t)valor; 
            break;
        case sizeof(uint32_t):
            *(uint32_t *)reg_a_setear = (uint32_t)valor;
            break;
    }
}

void SUM(registrosCPU registroDestino, registrosCPU registroOrigen){
    void *reg_destino = obtenerRegistro(registroDestino);
    void *reg_origen = obtenerRegistro(registroOrigen);

    if (reg_destino == NULL || reg_origen == NULL) {
        printf("Error: Uno o ambos registros no válidos.\n");
        return;
    }

    size_t tam_destino = tamanioRegistro(registroDestino);
    size_t tam_origen = tamanioRegistro(registroOrigen);


    switch (tam_destino) {  
        case sizeof(uint8_t):
            switch(tam_origen){
                case sizeof(uint8_t):
                    *(uint8_t *)reg_destino += *(uint8_t *)reg_origen; break;
                case sizeof(uint32_t):
                    *(uint8_t *)reg_destino += *(uint32_t *)reg_origen; break; 
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

void SUB(registrosCPU registroDestino, registrosCPU registroOrigen){ 
    void *reg_destino = obtenerRegistro(registroDestino);
    void *reg_origen = obtenerRegistro(registroOrigen);

    if (reg_destino == NULL || reg_origen == NULL) {
        printf("Error: Uno o ambos registros no válidos.\n");
        return;
    }

    size_t tam_destino = tamanioRegistro(registroDestino);
    size_t tam_origen = tamanioRegistro(registroOrigen);


    switch (tam_destino) { 
        case sizeof(uint8_t):
            switch(tam_origen){
                case sizeof(uint8_t):
                    *(uint8_t *)reg_destino -= *(uint8_t *)reg_origen; break;
                case sizeof(uint32_t):
                    *(uint8_t *)reg_destino -= *(uint32_t *)reg_origen; break; // Se puede hacer esto?
            }
            break;
        case sizeof(uint32_t):
            switch(tam_origen){
                case sizeof(uint8_t):
                    *(uint32_t *)reg_destino -= *(uint8_t *)reg_origen; break;
                case sizeof(uint32_t):
                    *(uint32_t *)reg_destino -= *(uint32_t *)reg_origen; break;
            }
            break;
    }
}

void JNZ(registrosCPU registro, int instruccion){
    void *reg= obtenerRegistro(registro);

    if (reg == NULL) {  
        printf("Error: Registro inválido.\n"); 
        return;
    }

    size_t tam_reg = tamanioRegistro(registro);

    switch (tam_reg) {
        case sizeof(uint8_t):
                if(*(uint8_t *)reg != 0){
                   SET(miCPU.PC, instruccion);
                }
            break;
        case sizeof(uint32_t):
                if(*(uint32_t *)reg != 0){
                   SET(miCPU.PC, instruccion);
                }
            break;
    }
}

/*
void RESIZE(int tamaño){
    
}
*/

void IO_GEN_SLEEP(char* interfaz,int unidades_de_trabajo){
    t_paquete* paquete = crear_paquete(ENVIAR_IO_GEN_SLEEP);

    t_interfaz_generica* interfaz_a_enviar;
    interfaz_a_enviar->nombre = interfaz;
    interfaz_a_enviar->unidades_trabajo = unidades_de_trabajo;
    
    agregar_PCB_a_paquete(paquete,pcb);
    agregar_interfaz_generica_a_paquete(paquete, interfaz_a_enviar);
    enviar_paquete(paquete, socket_kernel_d);
    eliminar_paquete(paquete);
    
    pthread_mutex_lock(&mutexInterrupt);
    hay_interrupcion = 0;
    pthread_mutex_unlock(&mutexInterrupt);
}


void EXIT(){
    enviar_pcb(INSTRUCCION_EXIT);
    pthread_mutex_lock(&mutexInterrupt);
    hay_interrupcion = 0;
    pthread_mutex_unlock(&mutexInterrupt);
}


void inicializar_registros_cpu() {
    SET(miCPU.AX, 0);
    SET(miCPU.BX, 0);
    SET(miCPU.CX, 0);
    SET(miCPU.DX, 0);
    SET(miCPU.EAX, 0);
    SET(miCPU.EBX, 0);
    SET(miCPU.ECX, 0);
    SET(miCPU.EDX, 0);
    SET(miCPU.SI, 0);
    SET(miCPU.DI, 0);
    SET(miCPU.PC, 0);
}