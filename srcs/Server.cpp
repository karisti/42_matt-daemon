#include "../includes/Server.hpp"


MD::Server::Server() {}

MD::Server::Server(const char *port): port(port) {}

MD::Server::Server(const MD::Server &other) { *this = other; }

MD::Server::~Server() {}

MD::Server &MD::Server::operator=(const MD::Server &other)
{
	if (this != &other)
	{
		this->port = other.port;
	}

	return *this;
}

/* -- Getters -- */
int				MD::Server::getSocket(void) const { return this->sSocket; }
std::string		MD::Server::getPort(void) const { return std::string(this->port); }

int MD::Server::create()
{
	if ((this->sSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return this->reporter.error("Error opening socket");

	/** Bind the Socket to any free IP / Port **/
	sockaddr_in hint;
	hint.sin_family = AF_INET;			 // IPv4 type
	hint.sin_port = htons(atoi(this->port)); // Little Endian (for bigger numbers) | Host To Network Short
	inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
	int yes = 1;

	if (setsockopt(this->sSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		return this->reporter.error("Error could not reuse address");

	if (bind(this->sSocket, (struct sockaddr *)&hint, sizeof(hint)) < 0)
		return this->reporter.error("Error binding socket");

	/** Listen for new connections **/
	if (listen(this->sSocket, SOMAXCONN) == -1)
		return this->reporter.error("Error listen");

	if (fcntl(this->sSocket, F_SETFL, O_NONBLOCK) < 0)
		return this->reporter.error("Error making server socket non blocking");

	// Initialize epoll instance
	if ((this->epollFd = epoll_create1(0)) == -1)
		return this->reporter.error("epoll_create1");

	// Register the listening socket for read (incoming connections)
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = this->sSocket;
	if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, this->sSocket, &ev) == -1)
		return this->reporter.error("epoll_ctl add server socket");

	this->reporter.log("Server created at port: " + std::string(this->port));

	return 0;
}
