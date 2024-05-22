#ifndef INTERRUPCIONES_H_
#define INTERRUPCIONES_H_

#include "main.h"

extern pthread_mutex_t mutexInterrupt;

void recibir_interrupcion();

#endif /* INTERRUPCIONES_H_ */