#include <iostream>
#include "../includes/Server.hpp"
#include "../includes/Daemon.hpp"


int main() {
	MD::Daemon daemon;
	daemon.daemonize();
	

	// MD::Server server("4242");

	// server.createNetwork();
	// server.loop();
	// server.terminateServer();

	return 0;
}
