#include "conexiones.h"

void esperarClientesIO(t_conexion_escucha* params) {
    while(1) {
        int* socket_cliente = malloc(sizeof(int));
        pthread_t hilo;

        *socket_cliente = esperar_cliente(params->socket_servidor, params->modulo);

        
        pthread_create(&hilo,
                        NULL,
                        (void*) administrarConexionIO,
                        socket_cliente);
        pthread_detach(hilo);
    }
}

void administrarConexionIO(int socket_cliente) {
    t_IOConectado* dispositivo = malloc(sizeof(t_IOConectado));

    // provisorio, seguramente esto cambie
    recv(socket_cliente, dispositivo->nombreDispositivo, sizeof(int), MSG_WAITALL);

    dispositivo->cola = queue_create();
    sem_init(&dispositivo->semaforo, 0, 0);

    while(recv(socket_cliente, /* no se */, sizeof(/* no se */), MSG_WAITALL) > 0) {
        sem_wait(&dispositivo->semaforo);

    }

    queue_destroy(dispositivo->cola);
    sem_destroy(&dispositivo->semaforo);
    free(dispositivo);
}