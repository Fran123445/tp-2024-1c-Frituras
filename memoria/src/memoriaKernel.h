#include <utils/serializacion.h>
#include <stdlib.h>
#include <string.h>
#include "estructuras.h"

extern pthread_mutex_t mutex_tablas_paginas;
t_proceso_memoria* creacion_proceso(int socket_kernel);
void abrir_archivo_path(int socket_kernel);
void finalizar_proceso(int socket_kernel);
