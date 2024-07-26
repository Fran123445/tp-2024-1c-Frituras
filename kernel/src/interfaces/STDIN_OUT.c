#include "STDIN_OUT.h"

t_solicitudIOSTDIN_OUT* solicitudIOSTDIN_OUT_create(PCB* proceso, t_buffer* buffer) {
    t_solicitudIOSTDIN_OUT* solicitud = malloc(sizeof(t_solicitudIOSTDIN_OUT));
    solicitud->proceso = proceso;
    t_list* direcciones = list_create();
    
    while(buffer->size > 0) {
        t_direccionMemoria* dir = malloc(sizeof(t_direccionMemoria));
        dir->direccion = buffer_read_uint32(buffer);
        dir->tamanio = buffer_read_uint32(buffer);

        list_add(direcciones, dir);
    }

    solicitud->direcciones = direcciones;

    return solicitud;
}

void liberarSolicitudSTDIN_OUT(t_solicitudIOSTDIN_OUT* solicitud) {
    enviarAExit(solicitud->proceso, INVALID_INTERFACE);
    free(solicitud->direcciones);
    free(solicitud);
}

void manejarSTDINOUT(int* socket_cliente, t_IOConectada* interfaz) {
    op_code op;

    while (1) {
        t_paquete* paquete = crear_paquete(CONEXION_STDOUT);
        sem_wait(&interfaz->semaforo);

        pthread_mutex_lock(&interfaz->mutex);
        t_solicitudIOSTDIN_OUT* solicitud = queue_pop(interfaz->procesosBloqueados);
        pthread_mutex_unlock(&interfaz->mutex);

        agregar_int_a_paquete(paquete, solicitud->proceso->PID);
        while(list_size(solicitud->direcciones) > 0 ) {
            t_direccionMemoria* dir = list_remove(solicitud->direcciones, 0);
            agregar_uint32_a_paquete(paquete, dir->direccion);
            agregar_uint32_a_paquete(paquete, dir->tamanio);
            free(dir);
        }

        enviar_paquete(paquete, *socket_cliente);
        eliminar_paquete(paquete);

        op = recibir_operacion(*socket_cliente);
        if (op <= 0) {
            log_error(logger, "La operación de IO STDIN/STDOUT no se pudo completar exitosamente");
            enviarAExit(solicitud->proceso, INVALID_INTERFACE);
            free(solicitud);
            break;
        }
        
        pthread_mutex_lock(&mutexPlanificador);
        planificar(op, solicitud->proceso, NULL);
        pthread_mutex_unlock(&mutexPlanificador);

        list_destroy_and_destroy_elements(solicitud->direcciones, free);
        free(solicitud);
    }

    queue_destroy_and_destroy_elements(interfaz->procesosBloqueados, (void *) liberarSolicitudSTDIN_OUT);
    liberarInterfazConectada(interfaz);
}

void administrarSTDIN(int* socket_cliente) {
    t_IOConectada* interfaz = IOConectado_create(*socket_cliente, INTERFAZ_STDIN);

    manejarSTDINOUT(socket_cliente, interfaz);
}

void administrarSTDOUT(int* socket_cliente) {
    t_IOConectada* interfaz = IOConectado_create(*socket_cliente, INTERFAZ_STDOUT);

    manejarSTDINOUT(socket_cliente, interfaz);
}