#pragma once

#include <string>
#include <iostream>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>

#include "Exception.hpp"


namespace MD
{
	class Tintin_reporter
	{
		public:
			~Tintin_reporter();
			static Tintin_reporter& getInstance(const std::string &log_path, const std::string &reporter);
			int log(const std::string &message, const std::string &level = "INFO");
			int error(const std::string &message, bool raiseException = false);

		private:
			Tintin_reporter();
			Tintin_reporter(const std::string &log_path, const std::string &reporter);
			// Disallow copy and assignment
			Tintin_reporter(const Tintin_reporter&) = delete;
			Tintin_reporter& operator=(const Tintin_reporter&) = delete;

			void			openLogFile();
			int				createLogFile();
			std::string		getCurrentTimestamp();

		private:
			static Tintin_reporter*		instance;
			int							fd;
			std::string					log_path;
			std::string					reporter;
	};
}
