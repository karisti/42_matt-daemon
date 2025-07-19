#include "../includes/Daemon.hpp"
#include "../includes/utils.hpp"
#include "constants.hpp"

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
	// Check if the program is running as root
	if (geteuid() != 0)
	{
		std::cerr << "Este programa debe ejecutarse como root." << std::endl;
		exit(EXIT_FAILURE);
	}

	// // Check if the lock file already exists
	// const char *lock_path = "/var/lock/matt_daemon.lock";
	// if (access(lock_path, F_OK) == 0)
	// {
	// 	std::cerr << "Daemon is already running." << std::endl;
	// 	exit(EXIT_FAILURE);
	// }
}

void MD::Daemon::daemonize()
{
	this->create();
	this->run();
}

void MD::Daemon::create()
{
	umask(0);

	this->createFork();

	 // Create a new session
	pid_t sid = setsid();
	if (sid < 0)
	{
		std::cerr << "Failed to create new session" << std::endl;
		exit(EXIT_FAILURE);
	}

	this->createFork();
	this->lock();

	this->reporter.log("Daemon started with PID: " + std::to_string(getpid()) + ".");

	this->configSignals();
}

void MD::Daemon::run()
{
	// Daemon logic goes here
	this->reporter.log("Creating server...");

	MD::Server server("4242");

	server.createNetwork();

	this->reporter.log("Server created at port: " + std::string(server.getPort()));

	server.loop();
	server.terminateServer();

	this->reporter.log("Quitting.");
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
		exit(EXIT_SUCCESS);
	}

	return;
}

void MD::Daemon::stop()
{
	this->reporter.log("Stopping daemon.");

	// delete /var/lock/matt_daemon.lock
	const char *lock_path = "/var/lock/matt_daemon.lock";
	if (flock(fileno(lock_file), LOCK_UN) < 0)
	{
		this->reporter.log("Failed to unlock file: '" + std::string(lock_path) + "'", "ERROR");
		fclose(lock_file);
		exit(EXIT_FAILURE);
	}
	std::remove(lock_path);
}

void MD::Daemon::remove()
{
	this->reporter.log("Removing daemon.");
	// int cout_fd = open(STDOUT_FILENO, O_RDONLY);
	// close(cout_fd)
	// TODO: Implement the daemon removal logic
}

bool g_stopRequested = false;

void MD::Daemon::signalHandler(int signum)
{
	Tintin_reporter&		reporter = MD::Tintin_reporter::getInstance(LOG_PATH, LOG_REPORTER);
	reporter.log("Signal received: " + std::to_string(signum), "INFO");

	if (signum == SIGINT || signum == SIGTERM || signum == SIGQUIT || signum == SIGTSTP)
	{
		g_stopRequested = true;
	}
	else
	{
		reporter.log("Unknown signal received: " + std::to_string(signum), "ERROR");
	}
}

void MD::Daemon::configSignals()
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGTSTP, signalHandler);
}

void MD::Daemon::lock()
{
	if (chdir("/") < 0)
	{
		std::cerr << "Failed to change directory" << std::endl;
		exit(EXIT_FAILURE);
	}

	const char *lock_path = "/var/lock/matt_daemon.lock";
	this->lock_file = fopen(lock_path, "a");
	if (this->lock_file == NULL)
	{
		this->reporter.log("Failed to create lock file: '" + std::string(lock_path) + "'", "ERROR");
	}

	fprintf(this->lock_file, "%d", getpid());
	fflush(this->lock_file);

	if (flock(fileno(this->lock_file), LOCK_EX) < 0)
	{
		this->reporter.log("Failed to lock file: '" + std::string(lock_path) + "'", "ERROR");
		fclose(this->lock_file);
		exit(EXIT_FAILURE);
	}
}
