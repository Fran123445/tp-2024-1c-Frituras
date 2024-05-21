#ifndef INTERRUPCIONES_H_
#define INTERRUPCIONES_H_

#include <stddef.h> 
#include <stdio.h>
#include <utils/pcb.h>
#include <utils/serializacion.h>
#include <pthread.h>

void recibir_interrupcion(int socket_kernel);
int cambiar_variable_interrupcion(void);



#endif /* INTERRUPCIONES_H_ */