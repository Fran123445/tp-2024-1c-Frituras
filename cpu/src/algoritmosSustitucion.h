#ifndef algoritmosSustitucion_H_
#define algoritmosSustitucion_H_

#include "main.h"
#include "mmu.h"

extern t_queue* cola_FIFO;
extern t_list* estructura_LRU;

void FIFO(entrada_TLB* nueva_entrada_TLB);
void LRU(entrada_TLB* nueva_entrada_TLB);
void mover_al_frente_de_la_estructura_LRU(entrada_TLB* entrada);

#endif /* algoritmosSustitucion_H_ */