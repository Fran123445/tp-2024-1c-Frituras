#include <utils/serializacion.h>
#include "estructuras.h"

extern pthread_mutex_t mutex_bitarray_marcos_libres;

t_list* agregar_n_entradas_vacias(int cant_pags_a_agregar, t_list* tabla_del_proceso);
t_list* sacar_n_entradas_desde_final(int cant_pags_a_sacar, t_list* tabla_del_proceso);
void marcar_frames_como_libres(int cant_pags_a_sacar, t_list* tabla_del_proceso);
void chequear_espacio_memoria (int cant_pags, int socket_cpu);
void asignar_frames_a_paginas (int cant_pags_total, t_proceso_memoria* proceso);
t_proceso_memoria* recibir_proceso_cpu(int socket_cpu);
void mandar_instruccion_cpu(int socket_kernel, int socket_cpu);
void* resize_proceso(int socket_cpu);
void* acceso_tabla_paginas(int socket_cpu);

