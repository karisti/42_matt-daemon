#include "../includes/Server.hpp"
#include "../includes/Daemon.hpp"


void initialChecks()
{
	// Check if the program is running as root
	if (geteuid() != 0)
		throw MD::Exception("This program must be run as root.");
}

int main() {
	MD::Daemon daemon;

	try
	{
		initialChecks();

		daemon.daemonize();

		while (g_stopRequested == 0)
		{
			MD::Server server(SERVER_PORT);
			server.create();
			server.loop();
			server.terminate();

			if (g_stopRequested == SIGHUP)
			{
				g_stopRequested = 0;
				daemon.restart();
			}
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}
