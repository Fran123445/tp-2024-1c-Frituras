#include <commons/bitarray.h>
#include "memoriaContigua.h"

void* memoria_contigua;

void* iniciar_memoria (t_config* config) {
    void* memoria_contigua = malloc(tam_memoria);
    if(memoria_contigua == NULL){
        fprintf(stderr,"Error al asignar memoria");
        return NULL;
    }
    return memoria_contigua;
}

int calcular_marcos (t_config* config){
    int cant_marcos = tam_memoria/tam_pagina;
    return cant_marcos;
}

