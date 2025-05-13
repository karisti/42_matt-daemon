#include "../includes/Daemon.hpp"
#include "../includes/utils.hpp"


MD::Daemon::Daemon()
{
	this->creationTimestamp = getCurrentTimestamp();
}

MD::Daemon::~Daemon()
{
	this->stop();
	this->remove();
}

void MD::Daemon::daemonize()
{
	this->create();
	this->run();
}

void MD::Daemon::create()
{
	writeLog("Creating daemon...");
	
	pid_t pid, sid;

	pid = fork();
	if (pid < 0)
	{
		std::cerr << "Fork failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	if (pid > 0)
	{
		std::cout << "Daemon PID: " << pid << std::endl;
		exit(EXIT_SUCCESS);
	}
	sid = setsid();
	if (sid < 0)
	{
		std::cerr << "Failed to create new session" << std::endl;
		exit(EXIT_FAILURE);
	}
	if (chdir("/") < 0)
	{
		std::cerr << "Failed to change directory" << std::endl;
		exit(EXIT_FAILURE);
	}
	umask(0);
	close(STDIN_FILENO);
	// close(STDOUT_FILENO);
	close(STDERR_FILENO);

	writeLog("Daemon created successfully");
}

void MD::Daemon::run()
{
	writeLog("Running daemon...");
	// TODO: Implement the daemon running logic
}

void MD::Daemon::stop()
{
	writeLog("Stopping daemon...");
	// TODO: Implement the daemon stopping logic
}

void MD::Daemon::remove()
{
	writeLog("Removing daemon...");
	// TODO: Implement the daemon removal logic
}

void MD::Daemon::writeLog(const std::string &message)
{
	std::cout << "Writing log: " << message << std::endl;
	
	int fd = open("matt_daemon.log", O_RDWR | O_CREAT | O_APPEND, 0644);
	if (fd < 0)
	{
		std::cerr << "Failed to open matt_daemon.log" << std::endl;
		exit(EXIT_FAILURE);
	}

	write(fd, message.c_str(), message.size());
	write(fd, "\n", 1);
}

void MD::Daemon::signalHandler(int signum)
{
	std::cout << "Signal received: " << signum << std::endl;
	// TODO: Implement the signal handling logic
}


