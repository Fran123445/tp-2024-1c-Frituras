#ifndef PLAN_H
#define PLAN_H

#include <stdlib.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <procesos.h>

extern int gradoMultiprogramacion;
extern t_log* logger; // no estoy seguro de que esto se vaya a quedar aca

extern t_queue* colaNew;
extern t_queue* colaReady;
extern t_queue* colaBlocked;
extern t_queue* colaExit;

#endif /* PLAN_H */