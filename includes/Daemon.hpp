#pragma once

#include <string>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#include <csignal>
#include <cstring>
#include <cerrno>

#include "constants.hpp"
#include "Tintin_reporter.hpp"
#include "Server.hpp"


namespace MD
{
	class Daemon
	{
		private:
			FILE				*lock_file = nullptr;
			std::string			creationTimestamp;
			Tintin_reporter&	reporter = MD::Tintin_reporter::getInstance(LOG_PATH, LOG_REPORTER);
	
		public:
			Daemon();
			Daemon(const Daemon& other);
			~Daemon();
			Daemon &operator=(const Daemon &other);

			void daemonize();

		private:
			void createFork();
			void lock();
			void stop();
			void configSignals();
			static void signalHandler(int signum);
	};
}
