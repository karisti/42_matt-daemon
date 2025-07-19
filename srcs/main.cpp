#include "../includes/Server.hpp"
#include "../includes/Daemon.hpp"


int main() {

	MD::Daemon daemon;
	MD::Server server(SERVER_PORT);

	server.create();

	daemon.daemonize();

	server.loop();
	server.terminate();

	return 0;
}
