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


namespace MD
{
	class Daemon
	{
		private:
			bool				isRunning = false;
			FILE				*lock_file = nullptr;
			Tintin_reporter&	reporter = MD::Tintin_reporter::getInstance(LOG_PATH, LOG_REPORTER);

		public:
			Daemon();
			Daemon(const Daemon& other);
			~Daemon();
			Daemon &operator=(const Daemon &other);

			void daemonize();
			void stop();
			void restart();

		private:
			void createFork();
			void lock();
			void configSignals();
			static void signalHandler(int signum);
	};
}
