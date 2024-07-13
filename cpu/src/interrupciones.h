#ifndef INTERRUPCIONES_H_
#define INTERRUPCIONES_H_

#include "main.h"

extern op_code cod_op_int;
extern pthread_mutex_t mutexInterrupt;

int recibir_interrupcion();

#endif /* INTERRUPCIONES_H_ */