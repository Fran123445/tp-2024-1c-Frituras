#ifndef CICLO_DE_INSTRUCCION_H_
#define CICLO_DE_INSTRUCCION_H_

#include <commons/log.h>
#include <pthread.h>
#include <utils/pcb.h>
#include <utils/serializacion.h>
#include <stdint.h>
#include "interrupciones.h"
#include "main.h"

PCB* recibir_pcb(int socket_kernel);
void enviar_pcb(int socket,op_code motivo,PCB* pcb);
t_instruccion* fetch(PCB* pcb, int socket_memoria);
void enviar_PC_a_memoria(int socket_memoria, uint32_t pc);
t_instruccion* obtener_instruccion_de_memoria(int socket_memoria);
void decode_execute(t_instruccion* instruccion);
int check_interrupt(PCB* pcb, int socket_kernel);
void realizar_ciclo_de_instruccion(PCB* pcb, int socket_memoria, int socket_kernel);

#endif /* CICLO_DE_INSTRUCCION_H_ */