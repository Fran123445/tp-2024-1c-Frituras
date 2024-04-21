#include <procesos.h>

void crearProceso(char* path) {
    PCB* nuevoPCB = malloc(sizeof(PCB));

    // poner un semaforo para sincronizar ultimoPIDAsignado

    nuevoPCB->PID = ultimoPIDAsignado+1;
    //Agregar el de program counter
    nuevoPCB->quantum = quantum;
    //Agregar el de registrosCPU

    ultimoPIDAsignado += 1;

    // aca tambien va un semaforo
    // tambien "si el grado de multiprogramacion lo permite, va a ready"
    list_add(colaNew, nuevoPCB);    
}