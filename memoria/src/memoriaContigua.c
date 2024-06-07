#include "memoriaContigua.h"

void** memoria_contigua = NULL;

void iniciar_memoria(int tamanio){
    memoria_contigua = (void**)malloc(tamanio*sizeof(void*));
    if (memoria_contigua == NULL){
        fprintf(stderr, "Error al iniciar memoria");
        exit(EXIT_FAILURE);
    }
}

