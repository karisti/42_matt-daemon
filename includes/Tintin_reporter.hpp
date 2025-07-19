#pragma once

#include <string>
#include <iostream>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>


namespace MD
{
	class Tintin_reporter
	{
		public:
			~Tintin_reporter();
			static Tintin_reporter& getInstance(const std::string &log_path, const std::string &reporter);
			void log(const std::string &message, const std::string &level = "INFO");
			int  error(const std::string &message);

		private:
			Tintin_reporter();
			Tintin_reporter(const std::string &log_path, const std::string &reporter);
			// Prohibir copia y asignación
			Tintin_reporter(const Tintin_reporter&) = delete;
			Tintin_reporter& operator=(const Tintin_reporter&) = delete;
			
			void			openLogFile();
			int				createLogFile();
			std::string		getCurrentTimestamp();

		private:
			static Tintin_reporter*		instance;
		int								fd;
			std::string					log_path;
			std::string					reporter;
	};
}
