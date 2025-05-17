#include "../includes/Daemon.hpp"
#include "../includes/utils.hpp"

MD::Daemon::Daemon()
{
	this->initialChecks();
}

MD::Daemon::~Daemon()
{
	this->stop();
	this->remove();
}

void MD::Daemon::initialChecks()
{
	if (geteuid() != 0) {
		std::cerr << "Este programa debe ejecutarse como root." << std::endl;
		exit(EXIT_FAILURE);
	}

	// Check if the lock file already exists
	const char *lock_path = "/var/lock/matt_daemon.lock";
	if (access(lock_path, F_OK) == 0) {
		std::cerr << "Daemon is already running." << std::endl;
		exit(EXIT_FAILURE);
	}
}

void MD::Daemon::daemonize()
{
	this->create();
	this->run();
}

void MD::Daemon::create()
{
	umask(0);
	this->reporter.create("/var/log/matt_daemon.log", "Matt_daemon");

	this->createFork();
	
	pid_t sid = setsid(); // Create a new session
	if (sid < 0)
	{
		std::cerr << "Failed to create new session" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	this->createFork();
	this->lock();
	this->signals();
}

void MD::Daemon::run()
{
	MD::Server server("4242");

	writeLog("Running daemon...");
	// Child process
	// while (!g_stopRequested)
	// {
		// Daemon logic goes here
		writeLog("Daemon is running...");
		server.createNetwork();
		server.loop();
	// }
	// server.terminateServer();
	writeLog("finishing, daemon with kill");
}

void MD::Daemon::createFork()
{
	pid_t child_pid;

	child_pid = fork();
	if (child_pid < 0) // Fork failed
	{
		std::cerr << "Fork failed" << std::endl;
		exit(EXIT_FAILURE);
	}
	if (child_pid > 0) // Parent process
	{
		std::cout << "Daemon PID: " << child_pid << std::endl;
		exit(EXIT_SUCCESS);
	}
	
	return;
}

void MD::Daemon::stop()
{
	writeLog("Stopping daemon...");
	// TODO: Implement the daemon stopping logic

	// delete /var/lock/matt_daemon.lock
	const char *lock_path = "/var/lock/matt_daemon.lock";
	std::remove(lock_path);
}

void MD::Daemon::remove()
{
	writeLog("Removing daemon...");
	// int cout_fd = open(STDOUT_FILENO, O_RDONLY);
	// close(cout_fd)
	// TODO: Implement the daemon removal logic
}

void MD::Daemon::writeLog(const std::string &message)
{
	this->reporter.log(message);
}

void MD::Daemon::signals()
{
	signal(SIGHUP, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	// signal(SIGKILL, signalHandler);
}

void MD::Daemon::signalHandler(int signum)
{
	std::cout << "Signal received: " << signum << std::endl;
	
	if (signum == SIGHUP)
	{
		std::cout << "Stopping daemon..." << std::endl;
		const char *lock_path = "/var/lock/matt_daemon.lock";
		std::remove(lock_path);
		exit(EXIT_SUCCESS);
	} else if (signum == SIGINT || signum == SIGTERM || signum == SIGKILL) {
		std::cout << "Kill Signal recieved: " << signum << std::endl;
		g_stopRequested = 1;
	} else
	{
		std::cout << "Unknown signal received: " << signum << std::endl;
	}
}

void MD::Daemon::lock()
{
	if (chdir("/") < 0)
	{
		std::cerr << "Failed to change directory" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	const char *lock_path = "/var/lock/matt_daemon.lock";
	FILE *lock_file = fopen(lock_path, "a");

	writeLog("Locking the file...");

	if (lock_file == NULL) {
		std::cout << "There was an error creating the file " << std::endl;
	}

	fprintf(lock_file, "%d", getpid());
	fflush(lock_file);

	if (flock(fileno(lock_file), LOCK_EX) == 0) {
		std::cout << "The file was locked " << std::endl;
	} else {
		std::cout << "The file was not locked " << std::endl;
	}
}
