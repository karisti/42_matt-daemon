#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <vector>
#include <map>
#include "utils.hpp"

#define PING_TIMEOUT      100    // ms
#define REG_TIMEOUT       30     // sec
#define EPOLL_TIMEOUT     3000   // ms for epoll_wait
#define EPOLL_MAX_EVENTS  64
#define PRINT_DEBUG       1

namespace MD
{    
	class Server
	{
	public:
		// typedef std::map<int, User>                              users_map;
		// typedef std::vector<std::pair<users_map::iterator, std::string> >  user_map_iters_remove;
	
	private:
		std::string                   ip;
		int                           sSocket;
		int                           epollFd;
		std::string                   password;
		// struct epoll_event            event;          // temporary for epoll_ctl
		struct epoll_event            eventList[EPOLL_MAX_EVENTS];
		// users_map                     users;
		std::string                   hostname;
		std::string                   creationTimestamp;
	
	public:
		Server();
		Server(std::string password);
		Server(const Server &other);
		~Server();
		Server &operator=(const Server &other);
	
		/* -- Getters -- */
		std::string      getIp(void) const;
		int              getSocket(void) const;
		std::string      getPassword(void) const;
		// users_map&       getUsers(void);
		std::string      getHostname(void) const;
	
		/* -- Modifiers -- */
		void             setHostname(std::string hostname);
	
		/* -- Member functions -- */
		int              createNetwork();
		int              loop(void);  // inside loop use epoll_wait(...)
		// void             closeClient(User& user, std::string message);
		void             terminateServer(void);
		void             catchPing(void);
		
	private:
		/* -- Member functions -- */
		// void             removeUser(User& user);
		int              saveIp(void);
		// int              clientConnected(void);              // call epoll_ctl(ADD)
		// void             clientDisconnected(int eventFd);    // call epoll_ctl(DEL)
		// int              receiveMessage(int eventFd);
		// void             registration(User& user, std::string password);
		int              throwError(std::string message);
	};
}
