#pragma once

#include <string>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>


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
		
		void create();
		void run();
		void stop();
		void remove();
		void writeLog(const std::string &message);
		void signalHandler(int signum);
	};
}
