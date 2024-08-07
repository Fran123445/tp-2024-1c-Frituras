#include <planificacion/FIFO.h>

void enviarAIOFIFO(PCB* proceso, op_code operacion, t_buffer* buffer) {
    enviarAIO(proceso, operacion, buffer);
    cpuLibre = 1;
}

void operacionFinalizadaFIFO(PCB* proceso) {
    enviarAReady(proceso);
}

void waitFIFO(PCB* proceso, t_buffer* buffer) {
    cpuLibre = instruccionWait(proceso, buffer);
}

void signalFIFO(PCB* proceso, t_buffer* buffer) {
    cpuLibre = instruccionSignal(proceso, buffer);
}

void exitFIFO(PCB* proceso) {
    enviarAExit(proceso, SUCCESS);
    cpuLibre = 1;
}

void criterioEnvioFIFO() {
    if (cpuLibre && !queue_is_empty(colaReady)) {
        enviarProcesoACPU(sacarSiguienteDeReady());
        cpuLibre = 0;
    }
}

void setFIFO() {
    instIO = enviarAIOFIFO;
    IOFinalizada = operacionFinalizadaFIFO;
    instWait = waitFIFO;
    instSignal = signalFIFO;
    instExit = exitFIFO;
    interrupcion = NULL;
    criterioEnvio = criterioEnvioFIFO;
}