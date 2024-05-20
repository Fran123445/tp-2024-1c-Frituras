#ifndef CPU_H_
#define CPU_H_

#include <stddef.h> 
#include <stdio.h>
#include <utils/pcb.h>
#include <utils/serializacion.h>
#include <pthread.h>

int recibir_interrupcion(int socket_kernel);
cambiar_variable_interrupcion(void);



#endif /* CPU_H_ */