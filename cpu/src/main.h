#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h> 
#include <utils/pcb.h>
#include <utils/serializacion.h>
#include <utils/server.h>
#include <utils/client.h>

extern t_log* log_cpu;
extern PCB* pcb;
extern int socket_memoria;
extern int socket_kernel_d;
extern int socket_kernel_i;
extern volatile int hay_interrupcion;

extern pthread_mutex_t mutexPCB;
extern int hayPCB;

extern char* algoritmoSustitucionTLB;
extern int cant_entradas_TLB;
extern int tamanio_pagina;

#endif /* MAIN_H_ */