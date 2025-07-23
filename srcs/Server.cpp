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
		this->sSocket = other.sSocket;
		this->epollFd = other.epollFd;

		for (int i = 0; i < EPOLL_MAX_EVENTS; ++i)
		{
			this->eventList[i] = other.eventList[i];
		}

		for (MD::Server::clients_map::const_iterator it = other.clients.begin(); it != other.clients.end(); ++it)
		{
			this->clients[it->first] = it->second;
		}
	}

	return *this;
}

/* -- Getters -- */
int				MD::Server::getSocket(void) const { return this->sSocket; }
std::string		MD::Server::getPort(void) const { return std::string(this->port); }

void MD::Server::create()
{
	this->reporter.log("Creating server...");

	if ((this->sSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		this->reporter.error("Error opening socket", true);

	/** Bind the Socket to any free IP / Port **/
	sockaddr_in hint;
	hint.sin_family = AF_INET;			 // IPv4 type
	hint.sin_port = htons(atoi(this->port)); // Little Endian (for bigger numbers) | Host To Network Short
	inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
	int yes = 1;

	if (setsockopt(this->sSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		this->reporter.error("Error could not reuse address", true);

	if (bind(this->sSocket, (struct sockaddr *)&hint, sizeof(hint)) < 0)
		this->reporter.error("Error binding socket", true);

	/** Listen for new connections **/
	if (listen(this->sSocket, SOMAXCONN) == -1)
		this->reporter.error("Error listen", true);

	if (fcntl(this->sSocket, F_SETFL, O_NONBLOCK) < 0)
		this->reporter.error("Error making server socket non blocking", true);

	// Initialize epoll instance
	if ((this->epollFd = epoll_create1(0)) == -1)
		this->reporter.error("epoll_create1", true);

	// Register the listening socket for read (incoming connections)
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = this->sSocket;
	if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, this->sSocket, &ev) == -1)
		this->reporter.error("epoll_ctl add server socket", true);

	this->reporter.log("Server created at port: " + std::string(this->port));
}

void MD::Server::loop(void)
{
	int newEvents;
	g_stopRequested = false;

	/** SERVER LOOP **/
	while (!g_stopRequested)
	{
		// epoll_wait timeout in milliseconds
		// EPOLL_TIMEOUT is already expressed in ms so no conversion needed
		int timeout_ms = EPOLL_TIMEOUT;

		// Wait for events on epollFd, filling the same eventList array
		if ((newEvents = epoll_wait(this->epollFd,
									this->eventList,
									EPOLL_MAX_EVENTS, // max events to return
									timeout_ms)) == -1)
		{
			if (!g_stopRequested)
				this->reporter.error("epoll_wait", true);
		}

		/** Kqueue events loop **/
		for (int i = 0; i < newEvents; i++)
		{
			int eventFd = this->eventList[i].data.fd;

			/** Client disconnected **/
			if (this->eventList[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
			{
				clientDisconnected(eventFd);
			}
			/** New client connected **/
			else if (eventFd == getSocket())
			{
				clientConnected();
			}
			/** New message from client **/
			else if (this->eventList[i].events & EPOLLIN)
			{
				receiveMessage(eventFd);
			}
		}
	}
}

int MD::Server::clientConnected(void)
{
	MD::Client client;

	bool maxClientsReached = this->clients.size() >= MAX_CLIENTS;

	if (client.startListeningSocket(this->sSocket, maxClientsReached) < 0)
		return -1;

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = client.getSocket();
	if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, client.getSocket(), &ev) == -1)
		return this->reporter.error("epoll_ctl add client socket");

	this->clients[client.getSocket()] = client;
	return 0;
}

void	MD::Server::clientDisconnected(int eventFd)
{
	this->reporter.log("Client disconnected: " + std::to_string(eventFd), "LOG");

	MD::Server::clients_map::iterator clientIt = this->clients.find(eventFd);
	if (clientIt == this->clients.end())
		return ;

	closeClient(clientIt->second);
}

int	MD::Server::closeClient(MD::Client& client)
{
	/** Close client socket **/
	if (close(client.getSocket()) == -1)
		return this->reporter.error("Error closing client socket: " + std::to_string(client.getSocket()));

	this->reporter.log("Client socket (" + std::to_string(client.getSocket()) + ") closed", "LOG");
	this->clients.erase(client.getSocket());

	return 0;
}

int MD::Server::receiveMessage(int eventFd)
{
	char buf[4096];
	int bytesRec;

	memset(buf, 0, 4096);

	/** Receive data **/
	if ((bytesRec = recv(eventFd, buf, 4096, 0)) == -1)
		return this->reporter.error("Error in recv().");

	/** Check if client exists **/
	MD::Server::clients_map::iterator found = this->clients.find(eventFd);
	if (found == this->clients.end())
		return this->reporter.error("Client not found in map for fd: " + std::to_string(eventFd));

	MD::Client &client = found->second;

	/** Manage client buffer and split commands **/
	std::string message(buf);
	message.erase(remove(message.begin(), message.end(), '\n'), message.end());

	// Manage connection close
	if (bytesRec == 0)
	{
		this->reporter.log("Client (" + std::to_string(client.getSocket()) + ") disconnected", "LOG");
		closeClient(client);
		return 0;
	}
	if (message == "quit")
	{
		this->reporter.log("Request quit from client (" + std::to_string(client.getSocket()) + ").");
		closeClient(client);
		g_stopRequested = true;
		return 0;
	}

	this->reporter.log("User (" + std::to_string(client.getSocket()) + ") input: " + message, "LOG");

	return 0;
}

void MD::Server::terminate(void)
{
	/** Close client sockets **/
	for (MD::Server::clients_map::iterator clientIt = this->clients.begin(); clientIt != this->clients.end(); ++clientIt)
	{
		/** Close client socket **/
		if (close(clientIt->second.getSocket()) < 0)
			this->reporter.error("Error closing client socket: " + std::to_string(clientIt->second.getSocket()));
		else
			this->reporter.log("Client socket (" + std::to_string(clientIt->second.getSocket()) + ") closed", "LOG");
	}

	/** Close server socket **/
	if (close(this->getSocket()) == -1)
		this->reporter.error("Server close error");
	else
		this->reporter.log("Server (" + std::to_string(getSocket()) + ") closed.", "LOG");
}
