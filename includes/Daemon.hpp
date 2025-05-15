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
#include "Tintin_reporter.hpp"


namespace MD
{
	class Daemon
	{
	public:

		Daemon();
		~Daemon();
		
		void daemonize();
		
	private:
		std::string creationTimestamp;
		Tintin_reporter reporter;
		
		void create();
		void createFork();
		void lock();
		void run();
		
		void writeLog(const std::string &message);
		
		void signals();
		static void signalHandler(int signum);

		void stop();
		void remove();
	};
}
