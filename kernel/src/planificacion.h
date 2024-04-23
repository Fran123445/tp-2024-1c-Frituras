#ifndef PLAN_H
#define PLAN_H

#include <stdlib.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <commons/log.h>

extern int gradoMultiprogramacion;
extern t_log* logger; // no estoy seguro de que esto se vaya a quedar aca

extern t_list* colaNew;
extern t_list* colaReady;
extern t_list* colaBlocked;
extern t_list* colaExit;

#endif /* PLAN_H */