#include "dialFS.h"

void liberarSolicitudDialFS(t_solicitudDIALFS* solicitud) {
    enviarAExit(solicitud->proceso, INVALID_INTERFACE);
    list_destroy(solicitud->direcciones);
    free(solicitud->nombreArchivo);
    free(solicitud);
}

void administrarDIALFS(int* socket_cliente) {
    t_IOConectada* interfaz = IOConectado_create(*socket_cliente, INTERFAZ_DIALFS);

    while (1) {
        t_paquete* paquete = crear_paquete(CONEXION_DIALFS); // es posible que nunca lo cambie por algo que tenga mas sentido
        sem_wait(&interfaz->semaforo);

        pthread_mutex_lock(&interfaz->mutex);
        t_solicitudDIALFS* solicitud = queue_pop(interfaz->procesosBloqueados);
        pthread_mutex_unlock(&interfaz->mutex);

        paquete->codigo_operacion = solicitud->operacion;
        agregar_int_a_paquete(paquete, solicitud->proceso->PID);
        agregar_string_a_paquete(paquete, solicitud->nombreArchivo);

        switch (solicitud->operacion) {
            case ENVIAR_DIALFS_DELETE:
                break;
            case ENVIAR_DIALFS_CREATE:
            case ENVIAR_DIALFS_TRUNCATE:
                agregar_uint32_a_paquete(paquete, solicitud->tamanio);
                break;
            case ENVIAR_DIALFS_READ:
            case ENVIAR_DIALFS_WRITE:
                while(list_size(solicitud->direcciones) > 0 ) {
                    t_infoArchivo* info = list_remove(solicitud->direcciones, 0);
                    agregar_uint32_a_paquete(paquete, info->direccion);
                    agregar_uint32_a_paquete(paquete, info->tamanio);
                    agregar_uint32_a_paquete(paquete, info->ubicacionPuntero);
                    free(info);
                }
                break;
            default:
                break;
        }

        enviar_paquete(paquete, *socket_cliente);
        eliminar_paquete(paquete);

        op_code op = recibir_operacion(*socket_cliente);
        if (op <= 0) {
            log_error(logger, "La operación de File System no se pudo completar exitosamente");
            enviarAExit(solicitud->proceso, INVALID_INTERFACE);
            free(solicitud);
            break;
        }
        
        pthread_mutex_lock(&mutexPlanificador);
        planificar(op, solicitud->proceso, NULL);
        pthread_mutex_unlock(&mutexPlanificador);

        list_destroy(solicitud->direcciones);
        free(solicitud->nombreArchivo);
        free(solicitud);
    } 

    queue_destroy_and_destroy_elements(interfaz->procesosBloqueados, (void *) liberarSolicitudDialFS);
    liberarInterfazConectada(interfaz);
}



t_solicitudDIALFS* solicitudDIALFS_create(PCB* proceso, op_code operacion, t_buffer* buffer) {
    t_solicitudDIALFS* solicitud = malloc(sizeof(t_solicitudDIALFS));
    solicitud->proceso = proceso;
    solicitud->operacion = operacion;
    solicitud->direcciones = list_create();

    switch (operacion) {
        case ENVIAR_DIALFS_CREATE:
        case ENVIAR_DIALFS_DELETE:
            solicitud->nombreArchivo = buffer_read_string(buffer);
            break;
        case ENVIAR_DIALFS_TRUNCATE:
            solicitud->nombreArchivo = buffer_read_string(buffer);
            solicitud->tamanio = buffer_read_uint32(buffer);
            break;
        case ENVIAR_DIALFS_WRITE:
        case ENVIAR_DIALFS_READ:
            solicitud->nombreArchivo = buffer_read_string(buffer);

            while(buffer->size > 0) {
                t_infoArchivo* dir = malloc(sizeof(t_infoArchivo));
                dir->ubicacionPuntero = buffer_read_uint32(buffer);
                dir->direccion = buffer_read_uint32(buffer);
                dir->tamanio = buffer_read_uint32(buffer);
                list_add(solicitud->direcciones, dir);
            }

            break;
        default:
            break;
    }

    return solicitud;
}