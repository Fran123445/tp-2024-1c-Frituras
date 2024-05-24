#ifndef CICLO_DE_INSTRUCCION_H_
#define CICLO_DE_INSTRUCCION_H_

#include "main.h"


PCB* recibir_pcb();
void enviar_pcb(op_code motivo);
char* fetch();
void enviar_PC_a_memoria(uint32_t pc);
char* obtener_instruccion_de_memoria();
t_instruccion* decode(char* instruccion);
void execute(t_instruccion* instruccion);
int check_interrupt();
void realizar_ciclo_de_instruccion();

#endif /* CICLO_DE_INSTRUCCION_H_ */