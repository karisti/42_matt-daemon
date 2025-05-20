#include "../includes/Server.hpp"

MD::Server::Server() {
	reporter.create("/var/log/matt_daemon.log", "Matt_daemon");
}

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

std::string		MD::Server::getIp(void) const { return this->ip; }
int				MD::Server::getSocket(void) const { return this->sSocket; }
std::string		MD::Server::getHostname(void) const { return this->hostname; }
std::string		MD::Server::getPort(void) const { return std::string(this->port); }

int MD::Server::createNetwork()
{
	if ((this->sSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return throwError("Error opening socket");

	/** Bind the Socket to any free IP / Port **/
	sockaddr_in hint;
	hint.sin_family = AF_INET;			 // IPv4 type
	hint.sin_port = htons(atoi(this->port)); // Little Endian (for bigger numbers) | Host To Network Short
	inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
	int yes = 1;

	if (setsockopt(this->sSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		return throwError("Error could not reuse address");

	if (bind(this->sSocket, (struct sockaddr *)&hint, sizeof(hint)) < 0)
		return throwError("Error binding socket");

	/** Listen for new connections **/
	if (listen(this->sSocket, SOMAXCONN) == -1)
		return throwError("Error listen");

	if (fcntl(this->sSocket, F_SETFL, O_NONBLOCK) < 0)
		return throwError("Error making server socket non blocking");

	// Initialize epoll instance
	if ((this->epollFd = epoll_create1(0)) == -1)
		return throwError("epoll_create1");

	// Register the listening socket for read (incoming connections)
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = this->sSocket;
	if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, this->sSocket, &ev) == -1)
		return throwError("epoll_ctl add server socket");

	if (saveIp() == -1)
		return -1;

	// std::cout << "--- IP: " << this->ip << " ---" << std::endl;
	// std::cout << "--- Port: " << this->port << " ---" << std::endl;

	return 0;
}

int MD::Server::loop(void)
{
	int newEvents;
	g_stopRequested = false;
	/** SERVER LOOP **/
	while (!g_stopRequested)
	{
		// epoll_wait timeout in milliseconds (reuse KQUEUE_TIMEOUT if desired)
		int timeout_ms = EPOLL_TIMEOUT * 1000;

		// Wait for events on epollFd, filling the same eventList array
		if ((newEvents = epoll_wait(this->epollFd,
									this->eventList,
									EPOLL_MAX_EVENTS, // max events to return
									timeout_ms)) == -1)
		{
			if (!g_stopRequested)
				return throwError("epoll_wait");
		}

		/** Kqueue events loop **/
		for (int i = 0; i < newEvents; i++)
		{
			int eventFd = this->eventList[i].data.fd;

			/** Client disconnected **/
			if (this->eventList[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
			{
				this->reporter.log("Client disconnected: " + std::to_string(eventFd), "LOG");
				clientDisconnected(eventFd);
			}
			/** New client connected **/
			else if (eventFd == getSocket())
			{
				this->reporter.log("New client connecting: " + std::to_string(eventFd), "LOG");
				clientConnected();
			}
			/** New message from client **/
			else if (this->eventList[i].events & EPOLLIN)
			{
				// this->reporter.log("New message from client: " + std::to_string(eventFd), "LOG");
				receiveMessage(eventFd);
			}
		}
	}
	return 0;
}

int MD::Server::saveIp(void)
{
	char host[256];
	struct hostent *hostEntry;

	if (gethostname(host, sizeof(host)) == -1)
		return throwError("Error getting ip: gethostname");

	hostEntry = gethostbyname(host);
	if (!hostEntry)
	{
		this->ip = "127.0.0.1";
		this->hostname = "localhost";
	}
	else
	{
		this->ip = inet_ntoa(*((struct in_addr *)hostEntry->h_addr_list[0]));

		// std::cout << ">> Hostname: " << hostEntry->h_name << std::endl;
		std::string s(hostEntry->h_name);
		this->hostname = s;
	}

	return 0;
}

int		MD::Server::throwError(std::string message)
{
	perror(message.c_str());
	return -1;
}

int MD::Server::clientConnected(void)
{
	MD::Client client;

	bool maxClientsReached = this->clients.size() >= MAX_CLIENTS;
	client.startListeningSocket(this->sSocket, maxClientsReached);
	if (maxClientsReached)
	{
		closeClient(client);
		return -1;
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = client.getSocket();
	if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, client.getSocket(), &ev) == -1)
		return throwError("epoll_ctl add client socket");

	this->clients[client.getSocket()] = client;
	return 0;
}

void	MD::Server::clientDisconnected(int eventFd)
{
	MD::Server::clients_map::iterator clientIt = this->clients.find(eventFd);
	if (clientIt == this->clients.end())
		return ;

	closeClient(clientIt->second);
}

void	MD::Server::closeClient(MD::Client& client)
{
	/** Close client socket **/
	if (close(client.getSocket()) == -1)
		throwError("Client close error");
	else
		this->reporter.log("Client (" + std::to_string(client.getSocket()) + ") closed", "LOG");

	this->clients.erase(client.getSocket());
}

int MD::Server::receiveMessage(int eventFd)
{
	char buf[4096];
	int bytesRec;

	memset(buf, 0, 4096);

	/** Receive data **/
	if ((bytesRec = recv(eventFd, buf, 4096, 0)) == -1)
	{
		this->reporter.log("Error in recv(). Quitting", "ERROR");
		return -1;
	}

	MD::Server::clients_map::iterator found = this->clients.find(eventFd);
	if (found == this->clients.end())
		return -1;

	MD::Client &client = found->second;

	/** Manage client buffer and split commands **/
	std::string message(buf);
	message.erase(remove(message.begin(), message.end(), '\n'), message.end());

	// Manage connection close
	if (message == "quit" || bytesRec == 0)
	{
		this->reporter.log("Request quit from client (" + std::to_string(client.getSocket()) + ").", "INFO");
		closeClient(client);
		g_stopRequested = true;
		return 0;
	}

	this->reporter.log("User (" + std::to_string(client.getSocket()) + ") input: " + message, "LOG");

	return 0;
}

void MD::Server::terminateServer(void)
{
	/** Close client sockets **/
	for (MD::Server::clients_map::iterator clientIt = this->clients.begin(); clientIt != this->clients.end(); ++clientIt)
	{
		if (close(clientIt->second.getSocket()) == -1)
			throwError("Client close error");
		else
			this->reporter.log("Client (" + std::to_string(clientIt->second.getSocket()) + ") closed", "LOG");
	}

	/** Close server socket **/
	if (close(getSocket()) == -1)
		throwError("Server close error");
	else
		this->reporter.log("Server (" + std::to_string(getSocket()) + ") closed", "LOG");
}
