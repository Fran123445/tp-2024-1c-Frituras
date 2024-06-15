#ifndef MEM_KERNEL_H
#define MEM_KERNEL_H

#include <stdlib.h>
#include <string.h>
#include <utils/serializacion.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include "estructuras.h"
#include "memoriaDeInstrucciones.h"
#include "memoriaCPU.h"
#include "estructuras.h"

extern pthread_mutex_t mutex_lista_procesos;
extern t_log* log_memoria;
extern int tiempo_retardo;
extern t_bitarray* mapa_de_marcos;

t_proceso_memoria* creacion_proceso(int socket_kernel);
void abrir_archivo_path(int socket_kernel);
void eliminar_proceso(int pid_proceso);
void frames_libres_por_fin_proceso(t_proceso_memoria* proceso_a_eliminar);
void finalizar_proceso(int socket_kernel);

#endif