#include "server.h"
#include "estructurasConexion.h"

t_log *loggerServ;

int iniciar_servidor(char *puerto, t_log *log)
{
	loggerServ = log;

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

	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	
	// Asociamos el socket a un puerto
	if (bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
		log_error(loggerServ, "Error bind");
	}


	// Escuchamos las conexiones entrantes
	if (listen(socket_servidor, MAXCONN) < 0) {
		log_error(loggerServ, "Error listen");
	}
	
	freeaddrinfo(servinfo);
	log_trace(loggerServ, "Listo para escuchar a mi cliente");
	return socket_servidor;
}

int esperar_cliente(int socket_servidor, modulo_code modulo)
{
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	size_t bytes;

	int handshake_recibido;
	int resultOk = 0;
	int resultError = -1;

	bytes = recv(socket_cliente, &handshake_recibido, sizeof(int), MSG_WAITALL);

	if (compararHandshake(modulo, handshake_recibido))
	{
		bytes = send(socket_cliente, &resultOk, sizeof(int), 0);
	}
	else
	{
		bytes = send(socket_cliente, &resultError, sizeof(int), 0);
		log_info(loggerServ, "Ups! Te confundiste");
	}

	log_info(loggerServ, "Se conecto un cliente!");

	return socket_cliente;
}