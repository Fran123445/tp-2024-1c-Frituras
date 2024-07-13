#include "generica.h"

void liberarSolicitudGenerica(t_solicitudIOGenerica* solicitud) {
    enviarAExit(solicitud->proceso, INVALID_INTERFACE);
    free(solicitud);
}

void administrarInterfazGenerica(int* socket_cliente) {

    t_IOConectada* interfaz = IOConectado_create(*socket_cliente, INTERFAZ_GENERICA);

    op_code op;

    while (1) {
        t_paquete* paquete = crear_paquete(CONEXION_IOGENERICA);
        sem_wait(&interfaz->semaforo);

        pthread_mutex_lock(&interfaz->mutex);
        t_solicitudIOGenerica* solicitud = queue_pop(interfaz->procesosBloqueados);
        pthread_mutex_unlock(&interfaz->mutex);

        agregar_int_a_paquete(paquete, solicitud->unidadesTrabajo);
        enviar_paquete(paquete, *socket_cliente);
        eliminar_paquete(paquete);

        op = recibir_operacion(*socket_cliente);
        if (op <= 0) {
            log_error(logger, "La operación de IO genérica no se pudo completar exitosamente");
            enviarAExit(solicitud->proceso, INVALID_INTERFACE);
            free(solicitud);
            break;
        }
        
        pthread_mutex_lock(&mutexPlanificador);
        planificar(op, solicitud->proceso, NULL);
        pthread_mutex_unlock(&mutexPlanificador);

        free(solicitud);
    }

    queue_destroy_and_destroy_elements(interfaz->procesosBloqueados, (void *) liberarSolicitudGenerica);
    liberarInterfazConectada(interfaz);
}

t_solicitudIOGenerica* solicitudIOGenerica_create(PCB* proceso, t_buffer* buffer) {
    t_solicitudIOGenerica* solicitud = malloc(sizeof(t_solicitudIOGenerica));
    solicitud->proceso = proceso;
    solicitud->unidadesTrabajo = buffer_read_int(buffer);

    return solicitud;
}