#include "pcb.h"

size_t tamanioRegistro(registrosCPU registro) {
    if (registro >= AX && registro <= DX) {
        return sizeof(uint8_t);
    } else {
        return sizeof(uint32_t);
    }
}

void liberar_pcb(PCB* pcb) {
    string_array_destroy(pcb->recursosAsignados);
    free(pcb);
}
