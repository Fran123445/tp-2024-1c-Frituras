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

void agregar_uint32_a_paquete(t_paquete* paquete, uint32_t valor) {
	agregar_a_paquete(paquete, &valor, sizeof(uint32_t));
}

void agregar_string_a_paquete(t_paquete* paquete, char* string) {
	agregar_a_paquete(paquete, string, strlen(string)+1);
}

void agregar_string_array_a_paquete(t_paquete* paquete, char** string_array) {
	agregar_int_a_paquete(paquete, string_array_size(string_array));

	void _agregar_string(char* string) {
		agregar_string_a_paquete(paquete, string);
	};

	string_iterate_lines(string_array, (void *) _agregar_string);
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

	agregar_int_a_paquete(paquete, pcb->PID);
	agregar_uint32_a_paquete(paquete, pcb->programCounter);
	agregar_int_a_paquete(paquete, pcb->quantum);
	agregar_int_a_paquete(paquete, pcb->estado);
	agregar_a_paquete(paquete, &pcb->registros, sizeof(registros_cpu));
	agregar_string_array_a_paquete(paquete, pcb->recursosAsignados);

}

void agregar_interfaz_generica_a_paquete(t_paquete* paquete, t_interfaz_generica* interfaz) {
    agregar_string_a_paquete(paquete, interfaz->nombre);
    agregar_int_a_paquete(paquete, interfaz->unidades_trabajo);
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
	// esto podria modificarlo, ya que le esta dando un tope al tamaño
	// de los strings que recibimos, pero tendria que modificar buffer_read y
	// en este momento por lo menos no tengo ganas
    char* string = malloc(sizeof(char)*128);

    buffer_read(buffer, string);

    return string;
}

char** buffer_read_string_array(t_buffer* buffer) {
	int size = buffer_read_int(buffer);
	char** string_array = string_array_new();

	for(int i = 0; i < size; i++) {
		char* string = buffer_read_string(buffer);
		string_array_push(&string_array, string);
	}

	return string_array;
}

PCB* buffer_read_pcb(t_buffer* buffer) {
	PCB* pcb = malloc(sizeof(PCB));

	pcb->PID = buffer_read_int(buffer);
	pcb->programCounter = buffer_read_uint32(buffer);
	pcb->quantum = buffer_read_int(buffer);
	pcb->estado = buffer_read_int(buffer);
	buffer_read(buffer, &pcb->registros);
	pcb->recursosAsignados = buffer_read_string_array(buffer);
	
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

t_interfaz_generica* buffer_read_interfaz_generica(t_buffer* buffer) {
    t_interfaz_generica* interfaz = malloc(sizeof(t_interfaz_generica));

    interfaz->nombre = buffer_read_string(buffer);
    interfaz->unidades_trabajo = buffer_read_int(buffer);

    return interfaz;
}

void liberar_buffer(t_buffer* buffer) {
	free(buffer->stream);
	free(buffer);
}

int recibir_operacion(int socket_cliente)
{
	op_code cod_op;
	int tamanioBuffer;


	ssize_t bytes = recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL);
	if(bytes > 0) {
		// revisa si el tamaño del buffer es 0, de ser asi lo saca del buffer del socket
		recv(socket_cliente, &tamanioBuffer, sizeof(int), MSG_PEEK);
		if(tamanioBuffer == 0) {
			recv(socket_cliente, &tamanioBuffer, sizeof(int), MSG_WAITALL);
		}

		return cod_op;
	}
	else
	{
		return close(socket_cliente);
	}
}
