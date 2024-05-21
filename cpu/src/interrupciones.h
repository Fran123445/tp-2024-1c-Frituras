#ifndef INTERRUPCIONES_H_
#define INTERRUPCIONES_H_

#include <stddef.h> 
#include <stdio.h>
#include <utils/pcb.h>
#include <utils/serializacion.h>
#include <pthread.h>

extern int hay_interrupcion;
extern pthread_mutex_t mutexInterrupt;

void recibir_interrupcion(int socket_kernel);

#endif /* INTERRUPCIONES_H_ */