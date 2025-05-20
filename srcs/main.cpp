#include "../includes/Daemon.hpp"


int main() {
	MD::Daemon daemon;
	daemon.daemonize();

	return 0;
}
