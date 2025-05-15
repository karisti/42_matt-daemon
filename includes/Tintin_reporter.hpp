#pragma once

#include <string>
#include <iostream>
#include <sys/file.h>
#include <unistd.h>
#include <stdlib.h>


namespace MD
{
	class Tintin_reporter
	{
		public:
			Tintin_reporter();
			~Tintin_reporter();
			void create(const char *log_path, const std::string &reporter);
			void log(const std::string &message, const std::string &level = "INFO");

		private:
			void			openLogFile();
			void 			createLogFile();
			std::string		getCurrentTimestamp();

		private:
			int	fd;
			const char 	*log_path;
			std::string reporter;
	};
}
