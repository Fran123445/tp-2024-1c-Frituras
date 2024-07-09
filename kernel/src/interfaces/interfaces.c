#include "interfaces.h"

void esperarClientesIO(t_conexion_escucha* params) {
    while(1) {
        int* socket_cliente = malloc(sizeof(int));
        pthread_t hilo;

        *socket_cliente = esperar_cliente(params->socket_servidor, params->modulo);

        if (*socket_cliente < 0) {
            free(socket_cliente);
            break;
        }

        void (*func)(int*);

        switch(recibir_operacion(*socket_cliente)) {
            case CONEXION_IOGENERICA:
                func = &administrarInterfazGenerica;
                break;
            case CONEXION_STDIN:
                func = &administrarSTDIN;
                break;
            case CONEXION_STDOUT:
                func = &administrarSTDOUT;
                break;
            case CONEXION_DIAL_FS:
                func = &administrarDIALFS;
            default:
                log_error(logger, "Conexión inválida de una interfaz");
                break;
        }
        
        pthread_create(&hilo,
                        NULL,
                        (void*) func,
                        socket_cliente);
        pthread_detach(hilo);
    }
}

t_IOConectada* IOConectado_create(int socket_cliente, tipoInterfaz tipo) {
    t_IOConectada* interfaz = malloc(sizeof(t_IOConectada));

    t_buffer* buffer = recibir_buffer(socket_cliente);

    interfaz->nombreInterfaz = buffer_read_string(buffer);
    interfaz->tipo = tipo;
    interfaz->procesosBloqueados = queue_create();
    pthread_mutex_init(&(interfaz->mutex), NULL);
    sem_init(&(interfaz->semaforo), 0, 0);

    pthread_mutex_lock(&mutexListaInterfaces);
    list_add(interfacesConectadas, interfaz);
    pthread_mutex_unlock(&mutexListaInterfaces);

    return interfaz;
}

bool comprobarOperacionValida(t_IOConectada* interfaz, op_code operacion) {

    bool opValida;

    switch (interfaz->tipo) {
        case INTERFAZ_GENERICA:
            opValida = operacion == ENVIAR_IO_GEN_SLEEP;
            break;
        case INTERFAZ_STDIN:
            opValida = operacion == ENVIAR_IO_STDIN_READ;
            break;
        case INTERFAZ_STDOUT:
            opValida = operacion == ENVIAR_IO_STDOUT_WRITE;
            break;
        case INTERFAZ_DIALFS:
            opValida = (operacion == ENVIAR_DIALFS_CREATE   ||
                        operacion == ENVIAR_DIALFS_DELETE   ||
                        operacion == ENVIAR_DIALFS_TRUNCATE ||
                        operacion == ENVIAR_DIALFS_READ     ||
                        operacion == ENVIAR_DIALFS_WRITE    );
            break;
        default:
            opValida = false;
            break;
    }

    return opValida;
}

t_IOConectada* hallarInterfazConectada(char* nombre) {

    bool _mismoNombre(t_IOConectada* interfaz) {
        return !strcmp(nombre, interfaz->nombreInterfaz);
    };

    pthread_mutex_lock(&mutexListaInterfaces);
    t_IOConectada* interfaz = list_find(interfacesConectadas, (void *) _mismoNombre);
    pthread_mutex_unlock(&mutexListaInterfaces);

    return interfaz;
}