#ifndef CICLO_DE_INSTRUCCION_H_
#define CICLO_DE_INSTRUCCION_H_

#include <commons/log.h>
#include <pthread.h>
#include <utils/pcb.h>
#include <utils/serializacion.h>
#include <stdint.h>
#include "interrupciones.h"
#include "main.h"

PCB* recibir_pcb();
void enviar_pcb(op_code motivo);
t_instruccion* fetch();
void enviar_PC_a_memoria(uint32_t pc);
t_instruccion* obtener_instruccion_de_memoria();
void decode_execute(t_instruccion* instruccion);
int check_interrupt();
void realizar_ciclo_de_instruccion();

#endif /* CICLO_DE_INSTRUCCION_H_ */