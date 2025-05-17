#include "../includes/Server.hpp"

MD::Server::Server() {}

MD::Server::Server(const char *port): port(port) {}

MD::Server::Server(const MD::Server &other) { *this = other; }

MD::Server::~Server() {}

MD::Server &MD::Server::operator=(const MD::Server &other)
{
	if (this != &other)
	{
		this->ip = other.ip;
		this->sSocket = other.sSocket;
		this->epollFd = other.epollFd;
		this->hostname = other.hostname;
		this->port = other.port;
	}

	return *this;
}

/* -- Getters -- */
std::string MD::Server::getIp(void) const { return this->ip; }
int MD::Server::getSocket(void) const { return this->sSocket; }
std::string MD::Server::getHostname(void) const { return this->hostname; }

/* -- Modifiers -- */
void MD::Server::setHostname(std::string hostname)
{
	this->hostname = hostname;
}

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

	std::cout << "--- IP: " << this->ip << " ---" << std::endl;
	std::cout << "--- Port: " << this->port << " ---" << std::endl;

	return 0;
}

int MD::Server::loop(void)
{
	int newEvents;

	/** SERVER LOOP **/
	catchSignal();
	while (!socketKiller)
	{
		// Replace kqueue wait with epoll_wait:
		{
			// epoll_wait timeout in milliseconds (reuse KQUEUE_TIMEOUT if desired)
			int timeout_ms = EPOLL_TIMEOUT * 1000;

			// Wait for events on epollFd, filling the same eventList array
			if ((newEvents = epoll_wait(this->epollFd,
										this->eventList,
										EPOLL_MAX_EVENTS, // max events to return
										timeout_ms)) == -1)
			{
				if (!socketKiller)
					return throwError("epoll_wait");
			}
		}

		/** Kqueue events loop **/
		for (int i = 0; i < newEvents; i++)
		{
			int eventFd = this->eventList[i].data.fd;

			/** Client disconnected **/
			if (this->eventList[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
				clientDisconnected(eventFd);
			/** New client connected **/
			else if (eventFd == getSocket())
				clientConnected();
			/** New message from client **/
			else if (this->eventList[i].events & EPOLLIN)
				receiveMessage(eventFd);
		}
		// catchPing();
	}
	return 0;
}

// void MD::Server::closeClient(MD::User &user, std::string message)
// {
// 	/** Remove user from joined channels and send QUIT message to them **/
// 	for (MD::Server::channels_map::iterator itChannel = user.getJoinedChannels().begin(); itChannel != user.getJoinedChannels().end(); itChannel++)
// 	{
// 		MD::Server::channels_map::iterator channel = this->channels.find(itChannel->second.getName());
// 		if (channel != this->channels.end())
// 		{
// 			channel->second.sendMessageToUsers(user, ":" + user.getNick() + " QUIT :" + message);
// 			channel->second.removeUser(this, user);
// 		}
// 	}

// 	/** Close client socket **/
// 	if (close(user.getSocket()) == -1)
// 		throwError("Client close error");
// 	else
// 		std::cout << "Client (" << user.getSocket() << ") closed" << std::endl;

// 	removeUser(user);
// }

/* -- Private Member functions */
// void MD::Server::removeUser(MD::User &user)
// {
// 	this->users.erase(user.getSocket());
// }

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

		std::cout << ">> Hostname: " << hostEntry->h_name << std::endl;
		std::string s(hostEntry->h_name);
		this->hostname = s;
	}

	return 0;
}

void MD::Server::terminateServer(void)
{
	// /** Close client sockets **/
	// for (MD::Server::users_map::iterator userIt = this->users.begin(); userIt != this->users.end(); ++userIt)
	// {
	// 	if (close(userIt->second.getSocket()) == -1)
	// 		throwError("Client close error");
	// 	else
	// 		std::cout << "Client (" << userIt->second.getSocket() << ") closed" << std::endl;
	// }

	/** Close server socket **/
	if (close(getSocket()) == -1)
		throwError("Server close error");
	else
		std::cout << "Server (" << getSocket() << ") closed" << std::endl;
}

int MD::Server::clientConnected(void)
{
	MD::User user;

	user.startListeningSocket(this->sSocket);

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = user.getSocket();
	if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, user.getSocket(), &ev) == -1)
		return throwError("epoll_ctl add client socket");

	this->users[user.getSocket()] = user;
	return 0;
}

// void MD::Server::clientDisconnected(int eventFd)
// {
// 	MD::Server::users_map::iterator userIt = this->users.find(eventFd);
// 	if (userIt == this->users.end())
// 		return;

// 	// closeClient(userIt->second, "Quit: Connection closed");
// }

// int MD::Server::receiveMessage(int eventFd)
// {
// 	char buf[4096];
// 	int bytesRec;

// 	memset(buf, 0, 4096);

// 	/** Receive data **/
// 	if ((bytesRec = recv(eventFd, buf, 4096, 0)) == -1)
// 	{
// 		std::cout << "Error in recv(). Quitting" << std::endl;
// 		return -1;
// 	}

// 	MD::Server::users_map::iterator found = this->users.find(eventFd);
// 	if (found == this->users.end())
// 		return -1;

// 	MD::User &user = found->second;

// 	/** Manage user buffer and split commands **/
// 	std::string message(buf);
// 	message.erase(remove(message.begin(), message.end(), '\r'), message.end());
// 	user.appendBuffer(message);

// 	if (PRINT_DEBUG)
// 		std::cout << "Command: '" << user.getBuffer() << "'" << std::endl;

// 	std::vector<std::string> messageSplit = splitString(user.getBuffer(), "\n");
// 	if (messageSplit.size() == 0)
// 		return 0;

// 	user.clearBuffer();

// 	if (messageSplit[messageSplit.size() - 1].size() == 0)
// 		messageSplit.erase(messageSplit.end() - 1);
// 	else
// 	{
// 		user.appendBuffer(messageSplit[messageSplit.size() - 1]);
// 		messageSplit.erase(messageSplit.end() - 1);
// 	}

// 	/** Manage each command **/
// 	for (std::vector<std::string>::iterator it = messageSplit.begin(); it != messageSplit.end(); it++)
// 	{
// 		if (!user.isAuthenticated())
// 			registration(user, *it);
// 		else
// 		{
// 			MD::Command cmd(*it);
// 			cmd.detectCommand(this, user);
// 		}

// 		if (PRINT_DEBUG)
// 		{
// 			printUsers(this->users);
// 			std::cout << std::endl
// 					  << "*** *** *** *** *** *** *** *** *** *** ***" << std::endl
// 					  << std::endl;
// 		}
// 	}

// 	messageSplit.clear();

// 	return 0;
// }

// void MD::Server::registration(MD::User &user, std::string message)
// {
// 	MD::Command cmd(message);
// 	cmd.detectCommand(this, user);

// 	/** Registration completed **/
// 	if (user.getPassword().size() > 0 && user.getNick().size() > 0 && user.getUser().size() > 0 && !user.isAuthenticated())
// 	{
// 		user.setAuthenticated(true);

// 		user.sendMessage(":" + this->getHostname() + " 001 " + user.getNick() + " :Welcome to the 42 MD Network, " + user.getNick() + "!" + user.getUser() + "@" + user.getHostname());
// 		user.sendMessage(":" + this->getHostname() + " 002 " + user.getNick() + " :Your host is MDserv, running version 1.0");
// 		user.sendMessage(":" + this->getHostname() + " 003 " + user.getNick() + " :This server was created " + this->creationTimestamp);
// 		user.sendMessage(":" + this->getHostname() + " 004 " + user.getNick() + " :MDserv 1.0 ositnmlvk iso");
// 		user.sendMessage(":" + this->getHostname() + " 005 " + user.getNick() + " :CASEMAPPING=<ascii> MAXTARGETS=1 CHANLIMIT=#:10 PREFIX=(ov)@+ :are supported by this server");
// 	}
// }

int MD::Server::throwError(std::string message)
{
	perror(message.c_str());
	return -1;
}

void MD::Server::catchPing(void)
{
	if (PRINT_DEBUG)
		std::cout << "> Catch Ping: " << std::endl;

	/** Auxiliary vector to store the iterators that need to be removed **/
	// MD::Server::user_map_iters_remove iteratorsToRemove;
	// for (MD::Server::users_map::iterator userIt = this->users.begin(); userIt != this->users.end(); ++userIt)
	// {
	// 	if (PRINT_DEBUG)
	// 		std::cout << userIt->second.getNick() << " (" << userIt->second.getSocket() << ") -> " << REG_TIMEOUT + userIt->second.getTimeout() - time(NULL) << "s" << std::endl;

	// 	if (userIt->second.isPinged() && time(NULL) - userIt->second.getTimeout() > PING_TIMEOUT)
	// 		iteratorsToRemove.push_back(std::pair<MD::Server::users_map::iterator, std::string>(userIt, "PING ERROR"));
	// 	else if (!userIt->second.isPinged() && time(NULL) - userIt->second.getTimeout() > REG_TIMEOUT)
	// 	{
	// 		if (!userIt->second.isAuthenticated())
	// 			iteratorsToRemove.push_back(std::pair<MD::Server::users_map::iterator, std::string>(userIt, "REGISTRATION TIMEOUT"));
	// 		else
	// 		{
	// 			userIt->second.setPingKey(pingGenerator(5));
	// 			userIt->second.setPingRequest(true);
	// 			userIt->second.sendMessage("PING " + userIt->second.getPingKey());
	// 		}
	// 	}
	// }

	/** Remove the iterators that need to be removed from the map **/
	// for (MD::Server::user_map_iters_remove::iterator it = iteratorsToRemove.begin(); it != iteratorsToRemove.end(); ++it)
	// 	closeClient(it->first->second, it->second);
}
