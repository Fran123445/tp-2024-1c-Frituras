#include "mmu.h"

t_list* TLB;

uint32_t numPagAux; // No soy muy fan de esto, pero no sé cómo hacer para implementar esta_en_la_TLB sino.


uint32_t obtener_numero_pagina(uint32_t direccion_logica){
	return floor(direccion_logica/tamanio_pagina);
}

uint32_t obtener_desplazamineto_pagina(uint32_t direccion_logica){
	return direccion_logica - obtener_numero_pagina(direccion_logica) * tamanio_pagina;
}

uint32_t recibir_marco(){
    op_code cod_op = recibir_operacion(socket_memoria);
    if(cod_op == ACCESO_TABLAS_PAGINAS){
        t_buffer* buffer = recibir_buffer(socket_memoria);
        uint32_t marco = buffer_read_int(buffer);
        liberar_buffer(buffer);
        return marco;
    }
    return NULL;
}

void pedir_marco(uint32_t numero_pagina){
    t_paquete* paquete = crear_paquete(ACCESO_TABLAS_PAGINAS);
    agregar_int_a_paquete(paquete, pcb->PID);
    agregar_uint32_a_paquete(paquete, numero_pagina);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}


bool esta_en_la_TLB(void* data) {
    entrada_TLB* entrada = (entrada_TLB*) data;
    return (entrada->id_proceso == pcb->PID) && (entrada->pagina == numPagAux);
}


uint32_t traducir_direccion_logica_a_fisica(uint32_t direccion_logica){

    uint32_t numero_pagina = obtener_numero_pagina(direccion_logica);
    uint32_t desplazamiento = obtener_desplazamineto_pagina(direccion_logica);

    numPagAux = numero_pagina; //Acá hay que checkear lo que puse arriba.
    entrada_TLB* entradaExistenteEnTLB = (entrada_TLB*) list_find(TLB, esta_en_la_TLB);

    if (entradaExistenteEnTLB != NULL) {
        log_info(log_cpu, "PID: %u - TLB HIT - Pagina: %u", pcb->PID, numero_pagina);
        log_info(log_cpu, "PID: %u - OBTENER MARCO - Página: %u - Marco: %u", pcb->PID, numero_pagina, entradaExistenteEnTLB->marco);

        if(strcmp(algoritmoSustitucionTLB, "LRU") == 0){
           mover_al_frente_de_la_estructura_LRU(entradaExistenteEnTLB);
        }

        return entradaExistenteEnTLB->marco * tamanio_pagina + desplazamiento;
    }
        else{
            free(entradaExistenteEnTLB);
        
            log_info(log_cpu, "PID: %u - TLB MISS - Pagina: %u", pcb->PID, numero_pagina);
    
            pedir_marco(numero_pagina);
            uint32_t marco = recibir_marco();

            entrada_TLB* entradaTLB = malloc(sizeof(entrada_TLB));
            entradaTLB->id_proceso = pcb->PID;
            entradaTLB->pagina = numero_pagina;
            entradaTLB->marco = marco;    

            if(list_size(TLB) < cant_entradas_TLB){
                list_add(TLB,entradaTLB);

                if(strcmp(algoritmoSustitucionTLB, "FIFO") == 0){
                    queue_push(cola_FIFO, entradaTLB);
                }
                else if(strcmp(algoritmoSustitucionTLB, "LRU") == 0){
                    list_add(estructura_LRU,entradaTLB);
                }
            }
            else{
                if(strcmp(algoritmoSustitucionTLB, "FIFO") == 0){
                    FIFO(entradaTLB);      
                }
                else if(strcmp(algoritmoSustitucionTLB, "LRU") == 0){
                    LRU(entradaTLB);
                }
            }           

            log_info(log_cpu, "PID: %u - OBTENER MARCO - Página: %u - Marco: %u", pcb->PID, numero_pagina, marco);

        return marco * tamanio_pagina + desplazamiento;
    }
}

