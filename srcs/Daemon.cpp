#include "../includes/Daemon.hpp"


MD::Daemon::Daemon(){}

MD::Daemon::Daemon(const MD::Daemon& other) { *this = other; }

MD::Daemon::~Daemon()
{
	if (this->isRunning)
		this->stop();
}

MD::Daemon& MD::Daemon::operator=(const MD::Daemon &other)
{
	if (this != &other)
	{
		this->lock_file = other.lock_file;
	}
	return *this;
}

void MD::Daemon::daemonize()
{
	this->lock();
	umask(0);

	this->createFork();

	// Create a new session
	pid_t sid = setsid();
	if (sid < 0)
		this->reporter.error("Failed to create new session", true);

	this->createFork();

	// Save the process ID to the lock file
	fprintf(this->lock_file, "%d", getpid());
	fflush(this->lock_file);

	this->reporter.log("Daemon started with PID: " + std::to_string(getpid()) + ".");
	this->isRunning = true;

	this->configSignals();
}

void MD::Daemon::createFork()
{
	pid_t child_pid;

	child_pid = fork();
	if (child_pid < 0) // Fork failed
		this->reporter.error("Failed to fork process", true);
	if (child_pid > 0) // Parent process
		exit(EXIT_SUCCESS);

	return;
}

void MD::Daemon::lock()
{
	if (chdir("/") < 0)
		this->reporter.error("Failed to change directory to root", true);

	const char *lock_path = LOCK_PATH;
	this->lock_file = fopen(lock_path, "a");
	if (this->lock_file == NULL)
		this->reporter.error("Failed to open lock file: '" + std::string(lock_path) + "'");

	if (flock(fileno(this->lock_file), LOCK_EX | LOCK_NB) < 0)
	{
		if (errno == EWOULDBLOCK)
		{
			// If we can't lock the file, it means another instance is running
			fclose(this->lock_file);
			this->reporter.error("Daemon is already running. '" + std::string(lock_path) + "' locked.", true);
		}
		else
		{
			fclose(this->lock_file);
			this->reporter.error("Failed to lock file: '" + std::string(lock_path) + "'", true);
		}
	}
}

void MD::Daemon::stop()
{
	this->reporter.log("Stopping daemon.");

	const char *lock_path = LOCK_PATH;
	if (flock(fileno(lock_file), LOCK_UN) < 0)
	{
		fclose(lock_file);
		this->reporter.error("Failed to unlock file: '" + std::string(lock_path) + "'", true);
	}
	std::remove(lock_path);
}

void MD::Daemon::restart()
{
	pid_t child_pid = fork();
	if (child_pid < 0) // Fork failed
		this->reporter.error("Failed to fork process for restart", true);
	if (child_pid > 0) // Parent process
	{
		this->reporter.log("Daemon restarting.");
		this->stop();
		exit(EXIT_SUCCESS);
	}

	// Child process
	this->daemonize();
}

int g_stopRequested = 0;
void MD::Daemon::signalHandler(int signum)
{
	Tintin_reporter&		reporter = MD::Tintin_reporter::getInstance(LOG_PATH, LOG_REPORTER);
	reporter.log("Signal received: " + std::to_string(signum));

	if (signum == SIGHUP || signum == SIGINT || signum == SIGQUIT || signum == SIGTERM)
	{
		g_stopRequested = signum;
	}
	else if (signum == SIGTSTP)
	{
		// SIGTSTP (Ctrl+Z) - Terminal Stop Signal
		// For a daemon, we log the signal but don't actually stop the process
		// as daemons should continue running in the background
		reporter.log("SIGTSTP received - Terminal stop signal ignored (daemon continues running)");
	}
	else if (signum == SIGCONT)
	{
		// SIGCONT - Continue Signal
		// This signal continues a stopped process
		// For a daemon, we just log that we received it and continue normal operation
		reporter.log("SIGCONT received - Continue signal processed (daemon continues normal operation)");
	}
	else
	{
		reporter.error("Signal " + std::to_string(signum) + " received but not handled.");
	}
}

void MD::Daemon::configSignals()
{
	signal(SIGHUP, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGTSTP, signalHandler);
	signal(SIGCONT, signalHandler);
}
