#pragma once

#include <iostream>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "Tintin_reporter.hpp"
#include "constants.hpp"

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
			int					socket;
			sockaddr_in			address;
			Tintin_reporter&	reporter = MD::Tintin_reporter::getInstance(LOG_PATH, LOG_REPORTER);

		public:
			int				getSocket(void) const;
			int				startListeningSocket(int serverSocket, bool maxClientsReached);
	};
}
