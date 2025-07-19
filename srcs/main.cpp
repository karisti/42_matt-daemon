#include "../includes/Server.hpp"


int main() {

	// Initialize the server with a specific port
	MD::Server server(SERVER_PORT);
	server.create();


	return 0;
}
