#ifndef MMU_H_
#define MMU_H_

#include "main.h"
#include <math.h>

extern t_queue* cola_FIFO;
extern t_list* estructura_LRU;

typedef struct {
    int id_proceso;
    uint32_t pagina;
    uint32_t marco;
}entrada_TLB;

extern t_list* TLB;

uint32_t obtener_desplazamineto_pagina(uint32_t direccion_logica);
uint32_t obtener_numero_pagina(uint32_t direccion_logica);
uint32_t traducir_direccion_logica_a_fisica(uint32_t direccion_logica);

#endif /* MMU_H_ */