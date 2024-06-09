#include <commons/bitarray.h>
#include "memoriaContigua.h"

void* memoria_contigua;

void* iniciar_memoria (t_config* config) {
    int tamanio_memoria = config_get_int_value(config, "TAM_MEMORIA");
    void* memoria_contigua = malloc(tamanio_memoria);
    if(memoria_contigua == NULL){
        fprintf(stderr,"Error al asignar memoria");
        return NULL;
    }
    return memoria_contigua;
}

int calcular_marcos (t_config* config){
    int tamanio_memoria = config_get_int_value(config, "TAM_MEMORIA");
    int tam_pag = config_get_int_value(config, "TAM_PAGINA");
    int cant_marcos = tamanio_memoria/tam_pag;
    return cant_marcos;
}

