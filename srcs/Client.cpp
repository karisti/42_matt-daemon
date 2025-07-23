#include "Client.hpp"


MD::Client::Client(void)
{
	this->socket = 0;
}

MD::Client::Client(const MD::Client& other) { *this = other; }
MD::Client::~Client() {}

MD::Client& MD::Client::operator=(const MD::Client &other)
{
	if (this != &other)
	{
		this->socket = other.socket;
		this->address = other.address;
	}

	return *this;
}

int	MD::Client::getSocket(void) const { return this->socket; }

int			MD::Client::startListeningSocket(int serverSocket, bool maxClientsReached)
{
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	memset(host, 0, NI_MAXHOST);
	memset(service, 0, NI_MAXSERV);

	socklen_t addressSize = sizeof(this->address);

	/** Accept client connection **/
	this->socket = accept(serverSocket, (struct sockaddr *)&this->address, &addressSize);
	if (this->socket == -1)
		return this->reporter.error("Accept socket error");

	/** Refuse connection if max clients reached **/
	if (maxClientsReached)
	{
		// Send message to client
		std::string message = "Connection refused. Max clients reached. Try later.\n";
		send(this->socket, message.c_str(), message.size(), 0);
		close(this->socket);

		this->reporter.log("Incoming client (" + std::to_string(this->socket) + "). Max clients reached. Connection refused.", "LOG");
		return -1;
	}

	/** Make non-blocking **/
	if (fcntl(this->socket, F_SETFL, O_NONBLOCK) < 0)
		return this->reporter.error("Error making client socket non blocking");

	this->reporter.log("New client connected: " + std::to_string(this->socket), "LOG");

	return 0;
}
