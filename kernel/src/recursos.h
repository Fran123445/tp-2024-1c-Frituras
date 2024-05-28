#ifndef RECS_H
#define RECS_H

#include <commons/collections/queue.h>

typedef struct {
    char* nombre;
    int instancias;
    t_queue* procesosBloqueados;
} t_recurso;

#endif /* RECS_H */