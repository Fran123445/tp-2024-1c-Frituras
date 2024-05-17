#include "interfaces.h"

void esperarClientesIO(t_conexion_escucha* params) {
    while(1) {
        int* socket_cliente = malloc(sizeof(int));
        pthread_t hilo;

        *socket_cliente = esperar_cliente(params->socket_servidor, params->modulo);

        void (*func)(int);

        switch(recibir_operacion(*socket_cliente)) {
            case CONEXION_IOGENERICA:
                func = &administrarInterfazGenerica;
                break;
            case CONEXION_STDIN:
                // A implementar
                break;
            case CONEXION_STDOUT:
                // A implementar
                break;
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

void administrarInterfazGenerica(int socket_cliente) {
    t_IOConectado* interfaz = malloc(sizeof(t_IOConectado));

    t_buffer* buffer = recibir_buffer(socket_cliente);

    interfaz->nombreInterfaz = buffer_read_string(buffer);
    interfaz->tipo = INTERFAZ_GENERICA;
    interfaz->cola = queue_create();
    pthread_mutex_init(&(interfaz->mutex), NULL);
    sem_init(&(interfaz->semaforo), 0, 0);

    list_add(interfacesConectadas, interfaz);

    t_solicitudIOGenerica* solicitud;

    while (1) {
        t_paquete* paquete = crear_paquete();
        sem_wait(&interfaz->semaforo);

        pthread_mutex_lock(&interfaz->mutex);
        solicitud = queue_pop(interfaz->cola);
        pthread_mutex_unlock(&interfaz->mutex);

        agregar_int_a_paquete(paquete, solicitud->unidadesTrabajo);
        enviar_paquete(paquete, socket_cliente);
        eliminar_paquete(paquete);

        if (recibir_operacion(socket_cliente) < 0) {
            log_error(logger, "La operación de IO genérica no se pudo completar exitosamente");
            //enviarAExit(proceso);
            break;
        }

        free(solicitud);
        //enviarAReady(proceso)
    } 
}

bool comprobarOperacionValida(t_IOConectado* interfaz, t_tipoInstruccion inst) {

    bool opValida;

    switch (interfaz->tipo) {
        case INTERFAZ_GENERICA:
            opValida = inst == IO_GEN_SLEEP;
            break;
        case INTERFAZ_STDIN:
            opValida = inst == IO_STDIN_READ;
        case INTERFAZ_STDOUT:
            opValida = inst == IO_STDOUT_WRITE;
        // Falta todo lo de FS
        default:
            opValida = false;
            break;
    }

    return opValida;
}

t_IOConectado* hallarInterfazConectada(char* nombre) {

    bool _mismoNombre(t_IOConectado* interfaz) {
        return !strcmp(nombre, interfaz->nombreInterfaz);
    };

    //hay que poner un mutex
    t_IOConectado* interfaz = list_find(interfacesConectadas, (void *) _mismoNombre);

    return interfaz;
}