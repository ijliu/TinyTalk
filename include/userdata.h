#pragma once

#include <iostream>

class UserData
{
public:
	UserData() :username(""), sockfd(-1) {}
	UserData(const std::string name, const int fd, const std::string ip) : username(name), sockfd(fd), addr(ip){}

	std::string getName();
	int getFD();
	std::string getAddr();
	void setName(const std::string name);
	void setFD(const int fd);
	void setAddr(const std::string addr);

	friend std::ostream& operator<<(std::ostream& os, const UserData& c1);
private:
	std::string username;
	int sockfd;
	std::string addr;
};