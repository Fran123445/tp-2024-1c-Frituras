#include "instrucciones.h"
#include <math.h>



int32_t traducir_direccion(uint32_t direccion_logica, t_pcb *pcb, uint32_t cantidad_bytes) 
{
    uint32_t número_página = obtener_numero_pagina(direccion_logica);
    uint32_t desplazamiento  = obtener_desplazamineto(direccion_logica);
    
    t_segmento segmento = pcb->tabla_segmentos[num_segmento]; 
    if((segmento.base + desplazamiento_segmento + cantidad_bytes) > segmento.limite){
        log_info(logger_cpu, "PID: %u - Error SEG_FAULT- Segmento: %u - Offset: %u - Tamaño: %u", pcb->pid, num_segmento, desplazamiento_segmento, segmento.limite);
        pcb->estado = SEG_FAULT;
        enviar_pcb(pcb, conexion_kernel, logger_cpu);
        temporal_stop(temporizador);
        enviar_contador(temporizador, conexion_kernel);
        temporal_destroy(temporizador);
        return -1;
    }
    return segmento.base + desplazamiento_segmento;
}


uint32_t obtener_numero_segmento(uint32_t direccion_logica) 
{
	return floor(direccion_logica/tamanio_pagina);
}


uint32_t obtener_desplazamineto_segmento(uint32_t direccion_logica) 
{
	return direccion_logica - numero_pagina * tamanio_pagina;
}