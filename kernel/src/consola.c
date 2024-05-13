#include "consola.h"

char* enumEstadoAString(estado_proceso estado) {
    char* string;

    // podria haber puesto un return en cada case, pero gcc me tiraba un warning
    // y me molestaba
    switch (estado)
    {
        case ESTADO_NEW: string = "NEW"; break;
        case ESTADO_READY: string = "READY"; break;
        case ESTADO_BLOCKED: string = "BLOCKED"; break;
        case ESTADO_EXEC: string = "EXEC"; break;
        case ESTADO_EXIT: string = "EXIT"; break;
    }

    return string;
}

void listarProcesos(void) {

    estado_proceso estadoAnterior = -1; // lo inicializo en -1 para que printee en la primera iteracion

    // ordena por estado y PID ascendente
    bool _criterioEstados(PCB* p1, PCB* p2) {
        return p1->estado <= p2->estado && p1->PID < p2->PID;
    }

    void _mostarProceso(PCB* proceso) {
        if (estadoAnterior != proceso->estado) {
            printf("\n%s\n\n", enumEstadoAString(proceso->estado));
        }

        printf("PID: %d\n", proceso->PID);
        estadoAnterior = proceso->estado;
    };

    pthread_mutex_lock(&mutexListaProcesos);
    t_list* listaOrdenadaPorEstado = list_sorted(listadoProcesos, (void *) _criterioEstados);
    pthread_mutex_unlock(&mutexListaProcesos);

    list_iterate(listaOrdenadaPorEstado, (void *) _mostarProceso);

    list_destroy(listaOrdenadaPorEstado);
}

void ejecutarScript(char* path) {
    FILE* archivoScript = fopen(path, "r");
    char* instruccion = malloc(sizeof(char)*64);

    while(fgets(instruccion, 63, archivoScript)) {
        instruccion[strlen(instruccion)-1] = '\0'; //fgets lee hasta, incluido, el \n
        interpretarInput(instruccion);
    }

    free(instruccion);
    fclose(archivoScript);
}

void interpretarInput(char* input) {

    char** comando = string_split(input, " ");

    // esto se podria hacer usando un diccionario y seguramente quedaria mucho mas lindo
    if (!strcmp(*comando, "EJECUTAR_SCRIPT")) {
        ejecutarScript(*(comando+1));
    } else if (!strcmp(*comando, "INICIAR_PROCESO")) {
        iniciarProceso("path");
    } else if (!strcmp(*comando, "FINALIZAR_PROCESO")) {
        finalizarProceso(atoi(*(comando+1)));
    } else if (!strcmp(*comando, "MULTIPROGRAMACION")) {
        //gradoMultiprogramacion =  atoi(*(comando+1)); <- esto hay que cambiarlo
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