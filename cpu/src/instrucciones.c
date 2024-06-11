#include "instrucciones.h"

void* obtenerRegistro(registrosCPU registro) {
    void* lista_de_registros[11] = {
        &pcb->registros.AX, &pcb->registros.BX, &pcb->registros.CX, &pcb->registros.DX,
        &pcb->registros.EAX, &pcb->registros.EBX, &pcb->registros.ECX, &pcb->registros.EDX,
        &pcb->registros.SI, &pcb->registros.DI, &pcb->registros.PC
    };

    if(registro >= 0 && registro < 11){
        return(lista_de_registros[registro]);
    }
    else{
        return NULL;
    }
}

void enviar_tamanio_resize_a_memoria(int tamanio_en_bytes){
    t_paquete* paquete = crear_paquete(ENVIO_RESIZE);
    agregar_int_a_paquete(paquete, pcb->PID);
    agregar_uint32_a_paquete(paquete, tamanio_en_bytes);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}

void pedir_contenido_memoria(uint32_t direccion_fisica, uint32_t tam){
    t_paquete* paquete = crear_paquete(ACCESO_ESPACIO_USUARIO_LECTURA);
    agregar_uint32_a_paquete(paquete, tam);
    agregar_uint32_a_paquete(paquete, direccion_fisica);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}

void* recibir_contenido_memoria(){
    op_code cod_op = recibir_operacion(socket_memoria);
    if(cod_op == ACCESO_ESPACIO_USUARIO_LECTURA){
        t_buffer* buffer = recibir_buffer(socket_memoria);
        void* data;
        buffer_read(buffer, data);
        liberar_buffer(buffer);
        return data;
    }
    return NULL;
}

void* contenido_obtenido_de_memoria(uint32_t direccionLogica, uint32_t tam){
    uint32_t direccion_fisica = traducir_direccion_logica_a_fisica(direccionLogica);
    pedir_contenido_memoria(direccion_fisica, tam);
    void* contenido_leido = recibir_contenido_memoria();  //le pido a memoria el contenido de la pagina
    void* puntero_al_dato_leido = &contenido_leido; // Si no uso esto para el memcpy me tira seg. fault

    return puntero_al_dato_leido;
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
                   SET(pcb->registros.PC, instruccion);
                }
            break;
        case sizeof(uint32_t):
                if(*(uint32_t *)reg != 0){
                   SET(pcb->registros.PC, instruccion);
                }
            break;
    }
}

void IO_GEN_SLEEP(char* interfaz,int unidades_de_trabajo){
    t_paquete* paquete = crear_paquete(ENVIAR_IO_GEN_SLEEP);

    t_interfaz_generica* interfaz_a_enviar = malloc(sizeof(t_interfaz_generica));
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

void RESIZE(int tamanio_en_bytes){
    enviar_tamanio_resize_a_memoria(tamanio_en_bytes);
    op_code cod_op = recibir_operacion(socket_memoria);
    if(cod_op == OUT_OF_MEMORY){
        enviar_pcb(cod_op);
    }
}

void MOV_IN(registrosCPU registroDatos, registrosCPU registroDireccion){

    void *reg_datos = obtenerRegistro(registroDatos);
    void *reg_direccion = obtenerRegistro(registroDireccion);

    size_t tamanio_a_leer = tamanioRegistro(registroDatos);

    uint32_t direccionLogicaInicial = *(uint32_t*)reg_direccion;

    uint32_t pagina_inicial = obtener_numero_pagina(direccionLogicaInicial);

    uint32_t pagina_final = obtener_numero_pagina(direccionLogicaInicial + tamanio_a_leer-1); //Porque, por ej, si son 4 bytes, lee del 30 al 33 (o sea, es leyendo el 30 incluído)

    if(tamanio_a_leer == 4){ // Hay que leer 4 bytes (Registros de 4 bytes)
        if(pagina_inicial == pagina_final){ //O sea, está en la misma página
            void* dato_leido = contenido_obtenido_de_memoria(direccionLogicaInicial, 4);
            memcpy(reg_datos, dato_leido, 4);
        }
        else{  // El contenido está en más de 1 página (como máximo dividido entre 2 páginas debido al tamaño de los registros y que las páginas tienen tamaño >1 y múltiplos de 2)
            int cant_bytes_a_leer_pagina_inicial = tamanio_pagina - (direccionLogicaInicial % tamanio_pagina);

            void* dato_leido1 = contenido_obtenido_de_memoria(direccionLogicaInicial, cant_bytes_a_leer_pagina_inicial);
            memcpy(reg_datos, dato_leido1, cant_bytes_a_leer_pagina_inicial);

            void* dato_leido2 = contenido_obtenido_de_memoria(direccionLogicaInicial + cant_bytes_a_leer_pagina_inicial, tamanio_a_leer - cant_bytes_a_leer_pagina_inicial); // A partir de donde terminó la lectura anterior lee lo que falta
            memcpy(reg_datos + cant_bytes_a_leer_pagina_inicial, dato_leido2, tamanio_a_leer - cant_bytes_a_leer_pagina_inicial);
        }
    }
    else{ // Hay que leer 1 byte (AX,BX,CX,DX)
        void* dato_leido = contenido_obtenido_de_memoria(direccionLogicaInicial, 1);
        memcpy(reg_datos, dato_leido, 1);
    }
    
}

void EXIT(){
    enviar_pcb(INSTRUCCION_EXIT);
    pthread_mutex_lock(&mutexInterrupt);
    hay_interrupcion = 0;
    pthread_mutex_unlock(&mutexInterrupt);
}
