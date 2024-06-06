#include "main.h"

int tamanio_pagina = 32; // Esto en realidad lo recibiriamos del handshake con memoria (o dsp), hay que hacerlo.


uint32_t obtener_numero_pagina(uint32_t direccion_logica) 
{
	return floor(direccion_logica/tamanio_pagina);
}

uint32_t obtener_desplazamineto_pagina(uint32_t direccion_logica) 
{
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


uint32_t traducir_direccion_logica_a_fisica(uint32_t direccion_logica){

    uint32_t numero_pagina = obtener_numero_pagina(direccion_logica);
    uint32_t desplazamiento = obtener_desplazamineto_pagina(direccion_logica);
    
    pedir_marco();
    int marco = recibir_marco();

    return marco * tamanio_pagina + desplazamiento;
}


