#pragma once

#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <unistd.h>
#include "utils.hpp"

#define EPOLL_TIMEOUT     3000   // ms for epoll_wait
#define EPOLL_MAX_EVENTS  3

namespace MD
{
	class Server
	{
		std::string				hostname;
		std::string				ip;
		const char				*port;
		int						sSocket;
		int						epollFd;
		struct epoll_event		eventList[EPOLL_MAX_EVENTS];
		
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

			/* -- Member functions -- */
			int		createNetwork();
			int		loop(void);
			
		private:
			/* -- Member functions -- */
			int		saveIp(void);
			int		throwError(std::string message);
	};
}

