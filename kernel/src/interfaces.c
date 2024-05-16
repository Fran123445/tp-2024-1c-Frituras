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

    // esto lo tengo que cambiar
    while(recv(socket_cliente, /* no se */, sizeof(/* no se */), MSG_WAITALL) > 0) {
        sem_wait(&interfaz->semaforo);
    }

    
}