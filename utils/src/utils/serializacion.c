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

t_paquete* crear_paquete(op_code operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = operacion;
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

void agregar_int_a_paquete(t_paquete* paquete, int valor) {
	agregar_a_paquete(paquete, &valor, sizeof(int));
}

void agregar_string_a_paquete(t_paquete* paquete, char* string) {
	agregar_a_paquete(paquete, string, strlen(string)+1);
}

void agregar_instruccion_a_paquete(t_paquete* paquete, t_instruccion* instruccion) {
	agregar_int_a_paquete(paquete, instruccion->tipo);
	agregar_a_paquete(paquete, instruccion->arg1, instruccion->sizeArg1);
	agregar_a_paquete(paquete, instruccion->arg2, instruccion->sizeArg2);
	agregar_a_paquete(paquete, instruccion->arg3, instruccion->sizeArg3);
	agregar_string_a_paquete(paquete, instruccion->interfaz);
	agregar_string_a_paquete(paquete, instruccion->archivo);
}

void agregar_PCB_a_paquete(t_paquete* paquete, PCB* pcb) {
	agregar_a_paquete(paquete, pcb, sizeof(PCB));
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

t_buffer* recibir_buffer(int socket_cliente){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);
	buffer->stream = malloc(buffer->size);
	recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

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
	buffer_read(buffer, pcb);

    return pcb;
}

t_instruccion* buffer_read_instruccion(t_buffer* buffer) {
	t_instruccion* inst = malloc(sizeof(t_instruccion));
	inst->arg1 = malloc(128);
	inst->arg2 = malloc(128);
	inst->arg3 = malloc(128);

	inst->tipo = buffer_read_int(buffer);
	buffer_read(buffer, inst->arg1);
	buffer_read(buffer, inst->arg2);
	buffer_read(buffer, inst->arg3);
	inst->interfaz = buffer_read_string(buffer);
	inst->archivo = buffer_read_string(buffer);

	return inst;
}

t_dispatch* buffer_read_dispatch(t_buffer* buffer) {
	t_dispatch* dispatch = malloc(sizeof(t_dispatch));
	dispatch->proceso = buffer_read_pcb(buffer);
	dispatch->instruccion = buffer_read_instruccion(buffer);

	return dispatch;
}

void liberar_buffer(t_buffer* buffer) {
	free(buffer->stream);
	free(buffer);
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}
