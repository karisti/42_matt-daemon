#include "../includes/Server.hpp"
#include "../includes/Daemon.hpp"


void initialChecks()
{
	// Check if the program is running as root
	if (geteuid() != 0)
	{
		std::cerr << "This program must be run as root." << std::endl;
		exit(EXIT_FAILURE);
	}
}

int main() {

	initialChecks();

	MD::Daemon daemon;
	MD::Server server(SERVER_PORT);

	server.create();

	daemon.daemonize();

	server.loop();
	server.terminate();

	return 0;
}
