#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <vector>
#include "userdata.h"

class TinyTalk
{
public:
	TinyTalk(int port);
	~TinyTalk();

	bool createTCPServer();
	int acceptUser(struct sockaddr_in &sa);
	void welcomeMSG(int fd);
	void exitMSG(int fd);
	void addUser(const int fd, const std::string ip);
	void run();
private:
	UserData* user;
	int port;
	int sockfd;
};
