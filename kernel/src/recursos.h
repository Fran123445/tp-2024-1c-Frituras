#ifndef RECS_H
#define RECS_H

#include <stdlib.h>
#include <stdio.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

typedef struct {
    char* nombre;
    int instancias;
    t_queue* procesosBloqueados;
} t_recurso;

extern t_list* listaRecursos;

void leerRecursosDeConfig(t_config* config);
t_recurso* hallarRecurso(char* nombre);

#endif /* RECS_H */