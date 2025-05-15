#include "../includes/Tintin_reporter.hpp"


MD::Tintin_reporter::Tintin_reporter() {
	std::cout << "Hello tintin..." << std::endl;
}
MD::Tintin_reporter::~Tintin_reporter() {
	std::cout << "Chao tintin..." << std::endl;
	close(this->fd);
}

void MD::Tintin_reporter::create(const char *log_path, const std::string &reporter)
{
	this->log_path = log_path;
	this->reporter = reporter;

	// Create the log file if it doesn't exist
	this->createLogFile();
	this->openLogFile();
	dup2(this->fd, STDOUT_FILENO);
	dup2(this->fd, STDERR_FILENO);
}

void MD::Tintin_reporter::createLogFile()
{
	int fd = open(this->log_path, O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
	{
		std::cerr << "Failed to create log file: " << this->log_path << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		close(fd);
	}
}

void MD::Tintin_reporter::openLogFile()
{
	int fd = open(this->log_path, O_WRONLY | O_APPEND);
	if (fd < 0)
	{
		std::cerr << "Failed to open log file: " << this->log_path << std::endl;
		exit(EXIT_FAILURE);
	}

	this->fd = fd;
}

void MD::Tintin_reporter::log(const std::string &message, const std::string &level)
{
	// Write the log message to the file
	std::string log_message = "[" + this->getCurrentTimestamp() + "] [ " + level + " ] - " + this->reporter + ": " + message + "\n";
	write(this->fd, log_message.c_str(), log_message.size());
}

std::string		MD::Tintin_reporter::getCurrentTimestamp()
{
	time_t now = time(0);
	struct tm* timeinfo = localtime(&now);
	char buffer[80];
	
	strftime(buffer, 80, "%d/%m/%Y-%H:%M:%S", timeinfo);
	return std::string(buffer);
}
