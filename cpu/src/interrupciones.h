#ifndef INTERRUPCIONES_H_
#define INTERRUPCIONES_H_

#include <stddef.h> 
#include <stdio.h>
#include <utils/pcb.h>
#include <utils/serializacion.h>
#include <pthread.h>
#include "main.h"

extern pthread_mutex_t mutexInterrupt;

void recibir_interrupcion();

#endif /* INTERRUPCIONES_H_ */