#pragma once

#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <map>
#include <algorithm>
#include "utils.hpp"
#include "Client.hpp"
#include "Tintin_reporter.hpp"

#define EPOLL_TIMEOUT     3000   // ms for epoll_wait
#define EPOLL_MAX_EVENTS  64
#define MAX_CLIENTS       3

namespace MD
{
	class Server
	{
		public:
			typedef std::map<int, Client>		clients_map;
		
		private:
			std::string				hostname;
			std::string				ip;
			const char				*port;
			int						sSocket;
			int						epollFd;
			struct epoll_event		eventList[EPOLL_MAX_EVENTS];
			clients_map				clients;
			Tintin_reporter&		reporter = MD::Tintin_reporter::getInstance();
		
		public:
			Server();
			Server(const char *port);
			Server(const Server &other);
			~Server();
			Server &operator=(const Server &other);

			/* -- Getters -- */
			std::string		getIp(void) const;
			int 			getSocket(void) const;
			std::string		getHostname(void) const;
			std::string		getPort(void) const;

			/* -- Member functions -- */
			int		createNetwork();
			int		loop(void);
			void	terminateServer(void);
			
		private:
			/* -- Member functions -- */
			int		saveIp(void);
			int		clientConnected(void);
			void	clientDisconnected(int eventFd);
			void	closeClient(Client& client);
			int		receiveMessage(int eventFd);
			int		throwError(std::string message);
	};
}

