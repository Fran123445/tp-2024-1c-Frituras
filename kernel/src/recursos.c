#include <recursos.h>

t_list* listaRecursos;

void leerRecursosDeConfig(t_config* config) {
    char** recursos = config_get_array_value(config, "RECURSOS");
    char** instancias = config_get_array_value(config, "INSTANCIAS_RECURSOS");

    listaRecursos = list_create();

    int size = string_array_size(recursos);

    for (int i = 0; i < size; i++) {
        t_recurso* recurso = malloc(sizeof(t_recurso));

        recurso->nombre = recursos[i];
        recurso->instancias = atoi(instancias[i]);
        recurso->procesosBloqueados = queue_create();

        list_add(listaRecursos, recurso);
    }

    string_array_destroy(instancias); // como hice atoi, no pasa nada liberando la memoria de los strings de las instancias
    free(recursos); // hago free en vez de array_destroy para destruir el array y no sus contenidos
}

t_recurso* hallarRecurso(char* nombre) {
    bool _mismoNombre(t_recurso* recurso) {
        return !strcmp(nombre, recurso->nombre);
    };

    // CREO que nunca van a haber varios hilos accediendo a la lista,
    // asi que no seria necesario un mutex
    t_recurso* recursoHallado = list_find(listaRecursos, (void *) _mismoNombre);

    return recursoHallado;
}

int waitRecurso(t_recurso* recurso, PCB* proceso) {
    // Me genera confusion que el enunciado diga estrictamente menor a 0
    // en vez de menor o igual a 0, ya que entonces un proceso podria tomar un
    // recurso que tiene 0 instancias ¿?
    int recursoTomado;
    if (recurso->instancias <= 0) {
        queue_push(recurso->procesosBloqueados, proceso);
        cambiarEstado(proceso, ESTADO_BLOCKED);

        char* str = string_new();
        string_append_with_format(&str, "BLOCKED %s", recurso->nombre);
        logProcesosEnCola(str, recurso->procesosBloqueados, false);
        free(str);

        string_array_push(&proceso->recursosAsignados, strdup(recurso->nombre));
        recursoTomado = 0;
    } else {
        string_array_push(&proceso->recursosAsignados, strdup(recurso->nombre));
        recursoTomado = 1;
    }
    
    recurso->instancias -= 1;

    return recursoTomado;
}

void sacarRecursoTomado(PCB* proceso, char* nombreRecurso) {
    char** recursos = string_array_new();
    char** recursosAsignadosActualmente = proceso->recursosAsignados;
    char* recurso;

    for(int i = 0; i < string_array_size(proceso->recursosAsignados); i++) {
        recurso = recursosAsignadosActualmente[i];
        if(strcmp(recurso, nombreRecurso)) {
            string_array_push(&recursos, recurso);
        }
    }

    free(recursosAsignadosActualmente);
    proceso->recursosAsignados = recursos;
}

void signalRecurso(t_recurso* recurso, PCB* proceso) {
    recurso->instancias += 1;

    sacarRecursoTomado(proceso, recurso->nombre);

    if(!queue_is_empty(recurso->procesosBloqueados)) {
        enviarAReady(queue_pop(recurso->procesosBloqueados));
    }
}

void liberarRecursos(PCB* proceso) {
    char** recursos = proceso->recursosAsignados;

    if (!*recursos) return;

    for(int i = string_array_size(recursos); i > 0; i--) {
        t_recurso* recurso = hallarRecurso(proceso->recursosAsignados[0]);
        signalRecurso(recurso, proceso);
    }
}