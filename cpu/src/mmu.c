#include "mmu.h"

int tamanio_pagina = 32; // Esto en realidad lo recibiriamos del handshake con memoria (o dsp), hay que hacerlo.

t_list* TLB;

uint32_t numPagAux; // No soy muy fan de esto, pero no sé cómo hacer para implementar esta_en_la_TLB sino.


uint32_t obtener_numero_pagina(uint32_t direccion_logica){
	return floor(direccion_logica/tamanio_pagina);
}

uint32_t obtener_desplazamineto_pagina(uint32_t direccion_logica){
	return direccion_logica - obtener_numero_pagina(direccion_logica) * tamanio_pagina;
}

int* recibir_marco(){
    op_code cod_op = recibir_operacion(socket_memoria);
    if(cod_op == ENVIO_MARCO){
        t_buffer* buffer = recibir_buffer(socket_memoria);
        int marco = buffer_read_int(buffer);
        liberar_buffer(buffer);
        return int;
    }
    return NULL;
}

void pedir_marco(){
    t_paquete* paquete = crear_paquete(ENVIO_MARCO);
    agregar_int_a_paquete(paquete, pcb->PID);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}

bool esta_en_la_TLB(void* data) {
    entrada_TLB* entrada = (entrada_TLB*) data;
    return (entrada->id_proceso == pcb->PID) && (entrada->pagina == numPagAux);
}


uint32_t traducir_direccion_logica_a_fisica(uint32_t direccion_logica){

    t_log* log_TLB = log_create("Cpu.log", "CPU", false, LOG_LEVEL_INFO);

    uint32_t numero_pagina = obtener_numero_pagina(direccion_logica);

    numPagAux = numero_pagina; //Acá hay que checkear lo que puse arriba.
    entrada_TLB* entradaExistenteEnTLB = (entrada_TLB*) list_find(TLB, esta_en_la_TLB);

    if (entradaExistenteEnTLB != NULL) {
        log_info(log_TLB, "PID: %u - TLB HIT - Pagina: %u", pcb->PID, numero_pagina);
        log_info(log_TLB, "PID: %u - OBTENER MARCO - Página: %u - Marco: %u", pcb->PID, numero_pagina, entradaExistenteEnTLB->marco);
        log_destroy(log_TLB);

        return entradaExistenteEnTLB->marco;
    }
        else{
            free(entradaExistenteEnTLB);
            log_info(log_TLB, "PID: %u - TLB MISS - Pagina: %u", pcb->PID, numero_pagina);
   
            entrada_TLB entradaTLB;

            uint32_t desplazamiento = obtener_desplazamineto_pagina(direccion_logica);
    
            pedir_marco();
            uint32_t marco = recibir_marco();

            entradaTLB->id_proceso = pcb->PID;
            entradaTLB->pagina = numero_pagina;
            entradaTLB->marco = marco;    

            if(list_size(TLB) < 32){
                list_add(TLB,entradaTLB);
            }
                else{
                    //Algoritmo de reemplazo para reemplazar en la TLB
                }           

            log_info(log_TLB, "PID: %u - OBTENER MARCO - Página: %u - Marco: %u", pcb->PID, numero_pagina, marco);
            log_destroy(log_TLB);

            return marco * tamanio_pagina + desplazamiento;
    }
}