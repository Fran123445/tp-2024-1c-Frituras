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
    t_recurso* recursoHallado = list_find(listaRecursos, _mismoNombre);

    return recursoHallado;
}