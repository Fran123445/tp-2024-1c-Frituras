#ifndef MMU_H_
#define MMU_H_

#include "main.h"

typedef struct {
    int id_proceso;
    uint32_t pagina;
    uint32_t marco;
} entrada_TLB;

extern t_list* TLB;


#endif /* MMU_H_ */