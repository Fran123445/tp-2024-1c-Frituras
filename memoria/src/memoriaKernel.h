#include <utils/serializacion.h>
#include <stdlib.h>
#include <string.h>
#include "estructuras.h"

extern pthread_mutex_t mutex_tablas_paginas;
t_proceso_memoria* creacion_proceso(int socket_kernel, int tiempo_retardo);
void abrir_archivo_path(int socket_kernel, int tiempo_retardo);
void eliminar_proceso(int pid_proceso);
void frames_libres_por_fin_proceso(t_proceso_memoria* proceso_a_eliminar);
void finalizar_proceso(int socket_kernel, int tiempo_retardo);