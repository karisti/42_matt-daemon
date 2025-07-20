#include "../includes/Server.hpp"
#include "../includes/Daemon.hpp"


void initialChecks()
{
	// Check if the program is running as root
	if (geteuid() != 0)
	{
		std::cerr << "This program must be run as root." << std::endl;
		throw MD::Exception("Must be run as root");
	}
}

int main() {
	try
	{
		initialChecks();

		MD::Daemon daemon;
		MD::Server server(SERVER_PORT);

		if (server.create() < 0)
			throw MD::Exception("Server creation failed");

		daemon.daemonize();

		server.loop();
		server.terminate();
	} catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}
