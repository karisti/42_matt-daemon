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
			~Tintin_reporter();
			static Tintin_reporter& getInstance();
			void create(const char *log_path, const std::string &reporter);
			void log(const std::string &message, const std::string &level = "INFO");

		private:
			Tintin_reporter();
			// Prohibir copia y asignación
			Tintin_reporter(const Tintin_reporter&) = delete;
			Tintin_reporter& operator=(const Tintin_reporter&) = delete;
			
			void			openLogFile();
			void 			createLogFile();
			std::string		getCurrentTimestamp();

		private:
			static Tintin_reporter* instance;
			int	fd;
			const char 	*log_path;
			std::string reporter;
	};
}
