#include <iostream>
#include "../includes/Server.hpp"


int main() {
	std::cout << "Hello, World!" << std::endl;
	

	MD::Server server("4242");

	server.createNetwork();
	server.loop();
	server.terminateServer();

	return 0;
}
