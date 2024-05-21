#ifndef CDI_H_
#define CDI_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <utils/pcb.h> 
#include <utils/serializacion.h>
#include <commons/log.h>
#include "instrucciones.h"
#include "interrupciones.h"

t_instruccion* fetch(PCB *pcb, int socket_memoria);
void enviar_PC_a_memoria(int socket_memoria,uint32_t pc);
t_instruccion* obtener_instruccion_de_memoria(int socket_memoria);
void decode_execute(t_instruccion* instruccion);

#endif /* CDI_H_ */