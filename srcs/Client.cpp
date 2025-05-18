#include "Client.hpp"


MD::Client::Client(void)
{
	this->socket = 0;
	this->hostname =  "";
	this->buffer = "";
}

MD::Client::Client(const MD::Client& other) { *this = other; }
MD::Client::~Client() {}

MD::Client& MD::Client::operator=(const MD::Client &other)
{
	this->address = other.address;
	this->socket = other.socket;
	this->hostname = other.hostname;
	this->buffer = other.buffer;
	
	return *this;
}

void			MD::Client::startListeningSocket(int serverSocket)
{
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	memset(host, 0, NI_MAXHOST);
	memset(service, 0, NI_MAXSERV);
	
	socklen_t addressSize = sizeof(this->address);
	
	/** Accept client connection **/
	this->socket = accept(serverSocket, (struct sockaddr *)&this->address, &addressSize);
	if (this->socket == -1)
	{
		perror("Accept socket error");
		return ;
	}

	/** Make non-blocking **/
	if (fcntl(this->socket, F_SETFL, O_NONBLOCK) < 0)
	{
		perror("Error making client socket non blocking");
		return ;
	}

	if (getnameinfo((struct sockaddr *) &this->address, addressSize, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
		std::cout << "Host1: " << host << " connected on port " << service << std::endl;
	else 
	{
		inet_ntop(AF_INET, &this->address.sin_addr, host, NI_MAXHOST);
		std::cout << "Host: " << host << " connected on port " << ntohs(this->address.sin_port) << std::endl;
	}
	
	std::string s(host);
	this->hostname = s;
}

int	MD::Client::getSocket(void) const { return this->socket; }
std::string	MD::Client::getHostname(void) const { return this->hostname; }

void			MD::Client::appendBuffer(std::string str) { this->buffer.append(str); }
void			MD::Client::clearBuffer(void) { this->buffer.clear(); }
std::string		MD::Client::getBuffer(void) const { return this->buffer; }
