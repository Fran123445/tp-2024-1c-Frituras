#include "instrucciones.h"
#include "interrupciones.h"

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

void enviar_tamanio_resize_a_memoria(uint32_t tamanio_en_bytes){
    t_paquete* paquete = crear_paquete(ENVIO_RESIZE);
    agregar_int_a_paquete(paquete, pcb->PID);
    agregar_uint32_a_paquete(paquete, tamanio_en_bytes);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}

void pedir_contenido_memoria(uint32_t direccion_fisica, uint32_t tam){
    t_paquete* paquete = crear_paquete(ACCESO_ESPACIO_USUARIO_LECTURA);
    agregar_uint32_a_paquete(paquete, direccion_fisica);
    agregar_uint32_a_paquete(paquete, tam);
    agregar_int_a_paquete(paquete, pcb->PID);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}

void* recibir_contenido_memoria(){
    op_code cod_op = recibir_operacion(socket_memoria);
    if(cod_op == ACCESO_ESPACIO_USUARIO_LECTURA){
        t_buffer* buffer = recibir_buffer(socket_memoria);
        void* data = NULL;
        buffer_read(buffer, data);
        liberar_buffer(buffer);
        return data;
    }
    return NULL;
}

void* contenido_obtenido_de_memoria(uint32_t direccion_fisica, uint32_t tam){
    pedir_contenido_memoria(direccion_fisica, tam);
    void* contenido_leido = recibir_contenido_memoria();  //le pido a memoria el contenido de la pagina
    // void* puntero_al_dato_leido = &contenido_leido; Me parece que esto ya no hace falta, veremos si hay seg. fault o no

    return contenido_leido;
}

void enviar_a_memoria_para_escritura(uint32_t direccion_fisica, void* datos_a_escribir, uint32_t tam) {
    t_paquete* paquete = crear_paquete(ACCESO_ESPACIO_USUARIO_ESCRITURA);
    agregar_uint32_a_paquete(paquete, direccion_fisica);
    agregar_uint32_a_paquete(paquete, tam);
    agregar_int_a_paquete(paquete, pcb->PID);
    agregar_a_paquete(paquete, datos_a_escribir,tam);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
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

void RESIZE(uint32_t tamanio_en_bytes){
    enviar_tamanio_resize_a_memoria(tamanio_en_bytes);
}

void MOV_IN(registrosCPU registroDatos, registrosCPU registroDireccion){

    void *reg_datos = obtenerRegistro(registroDatos);
    void *reg_direccion = obtenerRegistro(registroDireccion);

    size_t tamanio_a_leer = tamanioRegistro(registroDatos);

    uint32_t direccionLogicaInicial = *(uint32_t*)reg_direccion;

    uint32_t pagina_inicial = obtener_numero_pagina(direccionLogicaInicial);
    uint32_t pagina_final = obtener_numero_pagina(direccionLogicaInicial + tamanio_a_leer-1); //Porque, por ej, si son 4 bytes, lee del 30 al 33 (o sea, es leyendo el 30 incluído)

        if(pagina_inicial == pagina_final){ // Está en la misma página
            uint32_t direccionFisica = traducir_direccion_logica_a_fisica(direccionLogicaInicial);
            void* dato_leido = contenido_obtenido_de_memoria(direccionFisica, tamanio_a_leer);
            log_info(log_cpu, "Acción: LEER - Dirección física = %d - Valor: %d", direccionFisica, *(uint32_t*)dato_leido);
            memcpy(reg_datos, dato_leido, tamanio_a_leer);
        }
        else{ // El contenido está en más de 1 página
            uint32_t cant_paginas_a_leer = pagina_final - pagina_inicial + tamanio_a_leer; 
            int bytes_leidos = 0;
            
            for(int i=0; i<cant_paginas_a_leer; i++){
                int direccion_logica_actual = direccionLogicaInicial + bytes_leidos;
                uint32_t direccion_fisica_actual = traducir_direccion_logica_a_fisica(direccion_logica_actual);

                int cant_bytes_a_leer_pagina = tamanio_pagina - (direccion_logica_actual % tamanio_pagina); // En realidad calcula la cantidad de bytes restantes en la página desde la posición actual hasta el final de la página.
                if (cant_bytes_a_leer_pagina > (tamanio_a_leer - bytes_leidos)) { // Ajustar la cantidad de bytes a leer si es mayor que la cantidad restante (esto es para la última página, debido a lo que calcula en realidad la cuenta anterior)
                    cant_bytes_a_leer_pagina = tamanio_a_leer - bytes_leidos;
                }

                void* dato_leido = contenido_obtenido_de_memoria(direccion_fisica_actual, cant_bytes_a_leer_pagina);
                log_info(log_cpu, "Acción: LEER - Dirección física = %d - Valor: %d", direccion_fisica_actual, *(uint32_t*)dato_leido);
                memcpy(reg_datos + bytes_leidos, dato_leido, cant_bytes_a_leer_pagina);
                bytes_leidos += cant_bytes_a_leer_pagina;
            }
        }
}


void MOV_OUT(registrosCPU registroDireccion, registrosCPU registroDatos){

    void *reg_datos = obtenerRegistro(registroDatos);
    void *reg_direccion = obtenerRegistro(registroDireccion);
    uint32_t direccionLogicaInicial;

    size_t tamanio_a_escribir = tamanioRegistro(registroDatos);

    // Solucion (espero) provisional
    if (tamanioRegistro(registroDireccion) == sizeof(uint32_t)) {
        direccionLogicaInicial = *(uint32_t*)reg_direccion;
    } else {
        direccionLogicaInicial = *(uint8_t*)reg_direccion;
    }

    uint32_t pagina_inicial = obtener_numero_pagina(direccionLogicaInicial);
    uint32_t pagina_final = obtener_numero_pagina(direccionLogicaInicial + tamanio_a_escribir-1); 

    if(pagina_inicial == pagina_final){ 
        uint32_t direccion_fisica = traducir_direccion_logica_a_fisica(direccionLogicaInicial);
        log_info(log_cpu, "Acción: ESCRITURA - Dirección física = %d - Valor: %d", direccion_fisica, *(uint32_t*)reg_datos);
        enviar_a_memoria_para_escritura(direccion_fisica, reg_datos, tamanio_a_escribir);
    }
    else{ 
        uint32_t cant_paginas_a_escribir = pagina_final - pagina_inicial + 1; 
        int bytes_escritos = 0;

        for(int i=0; i<cant_paginas_a_escribir; i++){
            int direccion_logica_actual = direccionLogicaInicial + bytes_escritos;
            uint32_t direccion_fisica_actual = traducir_direccion_logica_a_fisica(direccion_logica_actual);
            int cant_bytes_a_escribir_pagina = tamanio_pagina - (direccion_logica_actual % tamanio_pagina);
            if (cant_bytes_a_escribir_pagina > (tamanio_a_escribir - bytes_escritos)) { 
                cant_bytes_a_escribir_pagina = tamanio_a_escribir - bytes_escritos;
            }
            log_info(log_cpu, "Acción: ESCRITURA - Dirección física = %d - Valor: %d", direccion_fisica_actual, *(uint32_t*)(reg_datos + bytes_escritos));
            enviar_a_memoria_para_escritura(direccion_fisica_actual, reg_datos + bytes_escritos, cant_bytes_a_escribir_pagina);
            bytes_escritos += cant_bytes_a_escribir_pagina;
        }
    }
}

void COPY_STRING(uint32_t tam){
    uint32_t direccion_logica_si = *(uint32_t *)obtenerRegistro(SI);
    uint32_t direccion_logica_di = *(uint32_t *)obtenerRegistro(DI);

    if (tam > tamanio_pagina){ // CHEQUEA SI LOS DATOS ESTAN Y HAY QUE ESCRIBIRLOS EN MAS DE UNA PAGINA
        
        uint32_t bytes_a_escribir = tam;

        while (bytes_a_escribir > 0){
            uint32_t tam_a_escribir = (bytes_a_escribir > tamanio_pagina) ? tamanio_pagina : bytes_a_escribir;

            uint32_t direccion_fisica_si = traducir_direccion_logica_a_fisica(direccion_logica_si);
            void *datos_de_si = contenido_obtenido_de_memoria(direccion_fisica_si, tam_a_escribir);
            log_info(log_cpu, "Acción: LEER - Dirección física = %d - Valor: %s", direccion_fisica_si, datos_de_si);

            uint32_t direccion_fisica_di = traducir_direccion_logica_a_fisica(direccion_logica_di);
            enviar_a_memoria_para_escritura(direccion_fisica_di, datos_de_si, tam_a_escribir);
            log_info(log_cpu, "Acción: ESCRIBIR - Dirección física = %d - Valor: %s", direccion_fisica_di, datos_de_si);

            bytes_a_escribir -= tam_a_escribir;
            direccion_logica_si += tam_a_escribir;
            direccion_logica_di += tam_a_escribir;
        }
    }
    else{// Si entra todo en una sola pagina
        uint32_t direccion_fisica_si = traducir_direccion_logica_a_fisica(direccion_logica_si);
        void *datos_de_si = contenido_obtenido_de_memoria(direccion_fisica_si, tam);
        log_info(log_cpu, "Acción: LEER - Dirección física = %d - Valor: %s", direccion_fisica_si, datos_de_si);

        uint32_t direccion_fisica_di = traducir_direccion_logica_a_fisica(direccion_logica_di);
        enviar_a_memoria_para_escritura(direccion_fisica_di, datos_de_si, tam);
        log_info(log_cpu, "Acción: ESCRIBIR - Dirección física = %d - Valor: %s", direccion_fisica_di, datos_de_si);
    }
}

void enviarDireccionesFisicasAKernel(char *interfaz, registrosCPU registroDireccion, registrosCPU registroTamaño, op_code operacion) {
    uint32_t tam = *(uint32_t *)obtenerRegistro(registroTamaño);
    uint32_t direccion_logica = *(uint8_t *)obtenerRegistro(registroDireccion);

    t_paquete* paquete = crear_paquete(operacion);
    agregar_PCB_a_paquete(paquete,pcb);
    agregar_string_a_paquete(paquete, interfaz);

    if (tam > tamanio_pagina){
        uint32_t bytes_a_enviar = tam;

        // Copiar los datos por partes a memoria
        while (bytes_a_enviar > 0){
            // Calcular la cantidad de bytes a copiar en esta parte
            uint32_t cant_de_bytes_a_enviar = (bytes_a_enviar > tamanio_pagina) ? tamanio_pagina : bytes_a_enviar;

            uint32_t direccion_fisica = traducir_direccion_logica_a_fisica(direccion_logica);

            agregar_uint32_a_paquete(paquete, direccion_fisica);
            agregar_uint32_a_paquete(paquete, cant_de_bytes_a_enviar);

            bytes_a_enviar -= cant_de_bytes_a_enviar;
            direccion_logica += cant_de_bytes_a_enviar;
        }
    }
    else{
        uint32_t direccion_fisica = traducir_direccion_logica_a_fisica(direccion_logica);
        agregar_uint32_a_paquete(paquete, direccion_fisica);
        agregar_uint32_a_paquete(paquete, tam);
    }

    enviar_paquete(paquete, socket_kernel_d);
    eliminar_paquete(paquete);
}

void IO_STDIN_READ(char *interfaz, registrosCPU registroDireccion, registrosCPU registroTamaño){
    enviarDireccionesFisicasAKernel(interfaz, registroDireccion, registroTamaño, ENVIAR_IO_STDIN_READ);

    pthread_mutex_lock(&mutexInterrupt);
    hay_interrupcion = 0;
    pthread_mutex_unlock(&mutexInterrupt);
}

void IO_STDOUT_WRITE(char *interfaz, registrosCPU registroDireccion, registrosCPU registroTamaño){
    enviarDireccionesFisicasAKernel(interfaz, registroDireccion, registroTamaño, ENVIAR_IO_STDOUT_WRITE);

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
