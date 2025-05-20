#pragma once

#include <iostream>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "Tintin_reporter.hpp"

namespace MD
{
	class Client
	{
		public:
			Client();
			Client(const Client& other);
			~Client();
			Client &operator=(const Client &other);

		private:
			sockaddr_in		address;
			int				socket;
			std::string		hostname;
			std::string		buffer;
			Tintin_reporter&		reporter = MD::Tintin_reporter::getInstance();

		public:
			/* -- Member functions -- */
			void			startListeningSocket(int serverSocket, bool maxClientsReached);
			int				getSocket(void) const;
			std::string		getHostname(void) const;

			std::string				getBuffer(void) const;
			void					appendBuffer(std::string str);
			void					clearBuffer(void);
	};
}
