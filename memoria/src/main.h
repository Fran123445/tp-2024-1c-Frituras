#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <pthread.h>
#include <semaphore.h>
#include "conexiones.h"
#include "estructuras.h"
#include "memoriaCPU.h"
#include "memoriaKernel.h"
#include "memoriaDeInstrucciones.h"
#include "memoriaContigua.h"

extern int socket_kernel;
extern int socket_cpu;
extern int socket_io;
extern int tiempo_retardo;
extern int tam_memoria;
extern int tam_pagina; 
extern t_config* config;
extern t_bitarray* mapa_de_marcos;
void* escuchar_kernel();
void* escuchar_cpu();


#endif /*MAIN_H */