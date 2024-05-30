#ifndef RECS_H
#define RECS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <utils/pcb.h>

typedef struct {
    char* nombre;
    int instancias;
    t_queue* procesosBloqueados;
} t_recurso;

extern t_list* listaRecursos;

void leerRecursosDeConfig(t_config* config);
t_recurso* hallarRecurso(char* nombre);

// Asigna un recurso a un proceso. Retorna 1 en caso de que el recurso este disponible
// y 0 si el proceso tuvo que ser bloqueado 
int waitRecurso(t_recurso* recurso, PCB* proceso);

#endif /* RECS_H */