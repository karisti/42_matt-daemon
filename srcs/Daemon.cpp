#include "../includes/Daemon.hpp"


MD::Daemon::Daemon()
{
	this->lock();
}

MD::Daemon::Daemon(const MD::Daemon& other) { *this = other; }

MD::Daemon::~Daemon()
{
	this->stop();
}

MD::Daemon& MD::Daemon::operator=(const MD::Daemon &other)
{
	if (this != &other)
	{
		this->lock_file = other.lock_file;
		this->creationTimestamp = other.creationTimestamp;
	}
	return *this;
}

void MD::Daemon::daemonize()
{
	umask(0);

	this->createFork();

	// Create a new session
	pid_t sid = setsid();
	if (sid < 0)
	{
		this->reporter.error("Failed to create new session");
		exit(EXIT_FAILURE);
	}

	this->createFork();

	// Save the process ID to the lock file
	fprintf(this->lock_file, "%d", getpid());
	fflush(this->lock_file);

	this->reporter.log("Daemon started with PID: " + std::to_string(getpid()) + ".");

	this->configSignals();
}

void MD::Daemon::createFork()
{
	pid_t child_pid;

	child_pid = fork();
	if (child_pid < 0) // Fork failed
	{
		this->reporter.error("Failed to fork process");
		exit(EXIT_FAILURE);
	}
	if (child_pid > 0) // Parent process
		exit(EXIT_SUCCESS);

	return;
}

void MD::Daemon::lock()
{
	if (chdir("/") < 0)
	{
		this->reporter.error("Failed to change directory to root");
		exit(EXIT_FAILURE);
	}

	const char *lock_path = LOCK_PATH;
	this->lock_file = fopen(lock_path, "a");
	if (this->lock_file == NULL)
		this->reporter.error("Failed to open lock file: '" + std::string(lock_path) + "'");

	if (flock(fileno(this->lock_file), LOCK_EX | LOCK_NB) < 0)
	{
		// If we can't lock the file, it means another instance is running
		if (errno == EWOULDBLOCK) {
			std::cerr << "Another instance of the daemon is already running." << std::endl;
			this->reporter.error("Daemon is already running. '" + std::string(lock_path) + "' locked.");
		}
		else {
			std::cerr << "Failed to lock file: '" << lock_path << "'" << std::endl;
			this->reporter.error("Failed to lock file: '" + std::string(lock_path) + "'");
		}

		fclose(this->lock_file);
		exit(EXIT_FAILURE);
	}
}

void MD::Daemon::stop()
{
	this->reporter.log("Stopping daemon.");

	const char *lock_path = LOCK_PATH;
	if (flock(fileno(lock_file), LOCK_UN) < 0)
	{
		this->reporter.error("Failed to unlock file: '" + std::string(lock_path) + "'");
		fclose(lock_file);
		exit(EXIT_FAILURE);
	}
	std::remove(lock_path);
}

bool g_stopRequested = false;
void MD::Daemon::signalHandler(int signum)
{
	Tintin_reporter&		reporter = MD::Tintin_reporter::getInstance(LOG_PATH, LOG_REPORTER);
	reporter.log("Signal received: " + std::to_string(signum));

	if (signum == SIGINT || signum == SIGTERM || signum == SIGQUIT || signum == SIGTSTP)
	{
		g_stopRequested = true;
	}
	else
	{
		reporter.error("Unknown signal received: " + std::to_string(signum));
	}
}

void MD::Daemon::configSignals()
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGTSTP, signalHandler);
}
