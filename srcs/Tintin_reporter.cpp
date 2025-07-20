#include "../includes/Tintin_reporter.hpp"

// Inicialización del puntero estático
MD::Tintin_reporter* MD::Tintin_reporter::instance = nullptr;

MD::Tintin_reporter::Tintin_reporter() {
}

MD::Tintin_reporter::Tintin_reporter(const std::string &log_path, const std::string &reporter)
{
	this->log_path = log_path;
	this->reporter = reporter;

	this->openLogFile();
	// dup2(this->fd, STDOUT_FILENO);
	// dup2(this->fd, STDERR_FILENO);
}

MD::Tintin_reporter::~Tintin_reporter() {
	close(this->fd);
}

MD::Tintin_reporter& MD::Tintin_reporter::getInstance(const std::string &log_path, const std::string &reporter) {
	if (instance == nullptr) {
		instance = new Tintin_reporter(log_path, reporter);
	}
	return *instance;
}

void MD::Tintin_reporter::openLogFile()
{
	int fd = open(this->log_path.c_str(), O_WRONLY | O_APPEND, 0644);
	if (fd < 0)
	{
		switch (errno)
		{
			case ENOENT:
				// Create the directory if it does not exist
				fd = this->createLogFile();
				break;
			case EACCES:
                                std::cerr << "Permission denied to open log file: " << this->log_path << std::endl;
                                throw MD::Exception("Permission denied to open log file");
                                break;
                        default:
                                std::cerr << "Failed to open log file: " << this->log_path << std::endl;
                                throw MD::Exception("Failed to open log file");
                                break;
                }
        }
	this->fd = fd;
}

int MD::Tintin_reporter::createLogFile() {

	// Create the directory structure if it does not exist
	std::string subdirs = this->log_path.substr(0, this->log_path.find_last_of('/'));
        if (mkdir(subdirs.c_str(), 0755) == -1 && errno != EEXIST) {
                std::cerr << "Failed to create directory: " << subdirs << std::endl;
                throw MD::Exception("Failed to create directory");
        }

	// Create the log file
	int fd = open(this->log_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) {
                std::cerr << "Failed to create log file: " << this->log_path << std::endl;
                throw MD::Exception("Failed to create log file");
        }

	return fd;
}

int MD::Tintin_reporter::log(const std::string &message, const std::string &level)
{
	// Write the log message to the file
	std::string log_message = "[" + this->getCurrentTimestamp() + "] [ " + level + " ] - " + this->reporter + ": " + message + "\n";
	write(this->fd, log_message.c_str(), log_message.size());
	return 0;
}

int MD::Tintin_reporter::error(const std::string &message, bool raiseException)
{
        this->log(message + ": " + std::strerror(errno), "ERROR");
        if (raiseException)
                throw MD::Exception(message);
        return -1;
}

std::string		MD::Tintin_reporter::getCurrentTimestamp()
{
	time_t now = time(0);
	struct tm* timeinfo = localtime(&now);
	char buffer[80];

	strftime(buffer, 80, "%d/%m/%Y-%H:%M:%S", timeinfo);
	return std::string(buffer);
}
