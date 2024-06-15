#include "main.h"

t_queue* cola_FIFO;
t_list* estructura_LRU;

void FIFO(entrada_TLB* nueva_entrada_TLB){
    entrada_TLB* entrada_TLB_a_sacar = (entrada_TLB*) queue_pop(cola_FIFO);

    list_remove_element(TLB, entrada_TLB_a_sacar);
    free(entrada_TLB_a_sacar);

    queue_push(cola_FIFO, nueva_entrada_TLB);

    list_add(TLB,nueva_entrada_TLB);
}

void LRU(entrada_TLB* nueva_entrada_TLB){
    entrada_TLB* entrada_TLB_a_sacar = (entrada_TLB*)list_get(estructura_LRU, list_size(estructura_LRU) - 1); //La que está a lo ultimo de la estructura_LRU, o sea, la que lleva más tiempo sin referenciarse.
    list_remove_element(TLB, entrada_TLB_a_sacar);

    list_remove_and_destroy_element(estructura_LRU, list_size(TLB) - 1, free); 
    list_add_in_index(estructura_LRU, 0, nueva_entrada_TLB); 

    list_add(TLB,nueva_entrada_TLB);
}


void mover_al_frente_de_la_estructura_LRU(entrada_TLB* entrada) {
    list_remove_element(estructura_LRU, entrada);
    list_add_in_index(estructura_LRU, 0, entrada);
}