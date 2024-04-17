#include "server.h"
#include "estructurasConexion.h"

t_log* logger;

int iniciar_servidor(char* puerto){
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	socket_servidor = socket(servinfo->ai_family,
                         servinfo->ai_socktype,
                         servinfo->ai_protocol);
						 
	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	
	// Escuchamos las conexiones entrantes

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");
	return socket_servidor;
}

void esperar_cliente(int socket_servidor,t_conexion_escucha* info){
	listen(socket_servidor, MAXCONN);
	while (1) {

		pthread_t thread;
		int *socket_cliente = malloc(sizeof(int));
		*socket_cliente = accept(socket_servidor, NULL, NULL);
		size_t bytes;

		int handshake_recibido;
		int resultOk = 0;
		int resultError = -1;

		bytes = recv(socket_cliente, &handshake_recibido, sizeof(int), MSG_WAITALL);

		if (info->handshake_escucha == handshake_recibido) 
		{
			bytes = send(socket_cliente, &resultOk, sizeof(int), 0);
		} else 
		{
			bytes = send(socket_cliente, &resultError, sizeof(int), 0);
			log_info(logger, "Ups! Te confundiste");
			free(socket_cliente)
			continue;
		}
		log_info(logger, "Se conecto un cliente!");
		pthread_create(&thread,
						NULL,
						(void*) atender_cliente,
						socket_cliente);
		pthread_detach(thread);
	}
}	

int recibir_operacion(int socket_cliente){
	int cod_op;
	ssize_t received = recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL);
	if(received > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente){
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente){
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente){
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

void* atender_cliente(int* cliente_fd) {
	t_list* lista;
    while (1) {
		int cod_op = recibir_operacion(*cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(*cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(*cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return -1;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}


int escucharConexiones(t_conexion_escucha* info) {
    

    char* puerto = config_get_string_value(info->config, info->puerto);

	logger = log_create(info->log, info->nombre_modulo, 1, LOG_LEVEL_DEBUG);

    int server_fd = iniciar_servidor(puerto);
    
	log_info(logger, "Servidor listo para recibir al cliente");
	esperar_cliente(server_fd,info);


	return 0;
}

