#include "serializacion.h"

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
    
    free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void* recibir_buffer(int* size, int socket_cliente){
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void buffer_read(t_buffer* buffer, void* data) {
    int size_data;
	memcpy(&size_data, buffer->stream, sizeof(int));
    memcpy(data, buffer->stream + sizeof(int), size_data);

    uint32_t nuevo_size = buffer->size - size_data - sizeof(int);
    void* nuevo_stream = malloc(nuevo_size);
    memcpy(nuevo_stream, buffer->stream + size_data + sizeof(int), nuevo_size);

    free(buffer->stream);
    buffer->size = nuevo_size;
    buffer->stream = nuevo_stream;
}

int buffer_read_int(t_buffer* buffer) {
    int data;
    buffer_read(buffer, &data);

    return data;
}

uint32_t buffer_read_uint32(t_buffer* buffer) {
    uint32_t data;
    buffer_read(buffer, &data);

    return data;
}

char* buffer_read_string(t_buffer* buffer) {
    char* string = malloc(sizeof(char)*128);

    buffer_read(buffer, string);

    return string;
}

PCB* buffer_read_pcb(t_buffer* buffer) {
    PCB* pcb = malloc(sizeof(PCB));
    pcb->PID = buffer_read_int(buffer);
    pcb->programCounter = buffer_read_uint32(buffer);
    pcb->quantum = buffer_read_int(buffer);
    pcb->estado = buffer_read_int(buffer);

    return pcb;
}