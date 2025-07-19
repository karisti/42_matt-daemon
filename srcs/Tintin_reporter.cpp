#include "../includes/Tintin_reporter.hpp"

// Inicialización del puntero estático
MD::Tintin_reporter* MD::Tintin_reporter::instance = nullptr;

MD::Tintin_reporter::Tintin_reporter() {
}

MD::Tintin_reporter::Tintin_reporter(const char *log_path, const std::string &reporter)
{
	this->log_path = log_path;
	this->reporter = reporter;

	this->openLogFile();
	dup2(this->fd, STDOUT_FILENO);
	dup2(this->fd, STDERR_FILENO);
}

MD::Tintin_reporter::~Tintin_reporter() {
	close(this->fd);
}

MD::Tintin_reporter& MD::Tintin_reporter::getInstance(const char *log_path, const std::string &reporter) {
	if (instance == nullptr) {
		instance = new Tintin_reporter(log_path, reporter);
	}
	return *instance;
}

void MD::Tintin_reporter::openLogFile()
{
	int fd = open(this->log_path, O_WRONLY | O_APPEND | O_CREAT, 0644);
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
