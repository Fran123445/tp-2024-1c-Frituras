#ifndef CICLO_DE_INSTRUCCION_H_
#define CICLO_DE_INSTRUCCION_H_

#include "main.h"

char* registro_a_string(registrosCPU registro);
t_tipoInstruccion string_a_tipo_instruccion (char* ins_char);
t_list* dividir_cadena_en_tokens(char* linea);
registrosCPU string_a_registro(const char* registro);
void liberar_instruccion(t_instruccion* instruccion);
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