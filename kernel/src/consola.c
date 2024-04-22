#include "consola.h"

void listarProcesos(void) {
    // Implementacion parcial. Falta agregar lo de los estados. 

    void _mostarProceso(PCB* proceso) {
        printf("PROCESO - %d\n", proceso->PID);
    };

    list_iterate(listadoProcesos, (void *) _mostarProceso);
}

void interpretarInput(char* input) {

    char** comando = string_split(input, " ");

    if (!strcmp(*comando, "EJECUTAR_SCRIPT")) {
        //ejecutarScript(path)
    } else if (!strcmp(*comando, "INICIAR_PROCESO")) {
        iniciarProceso("path");
    } else if (!strcmp(*comando, "FINALIZAR_PROCESO")) {
        finalizarProceso(atoi(*(comando+1)));
    } else if (!strcmp(*comando, "MULTIPROGRAMACION")) {
        //asignarNivelDeMultiprogramacion(nivel)
    } else if (!strcmp(*comando, "DETENER_PLANIFICACION")) {
        //detenerPlanificacion()
    } else if (!strcmp(*comando, "INICIAR_PLANIFICACION")) {
        //iniciarPlanificacion
    } else if (!strcmp(*comando, "PROCESO_ESTADO")) {
        listarProcesos();
    }

    string_array_destroy(comando);
}

void solicitarInput(void) {

    char* inputUsuario = readline(">");

    while(!string_is_empty(inputUsuario)) {
        interpretarInput(inputUsuario);
        free(inputUsuario);
        inputUsuario = readline(">");
    }

    free(inputUsuario);

}