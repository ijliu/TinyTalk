#include "userdata.h"

std::string UserData::getName()
{
	return username;
}

void UserData::setName(const std::string name)
{
	username = name;
}

int UserData::getFD()
{
	return sockfd;
}

void UserData::setFD(const int fd)
{
	sockfd = fd;
}

std::string UserData::getAddr()
{
	return addr;
}

void UserData::setAddr(const std::string ip)
{
	addr = ip;
}

std::ostream& operator<<(std::ostream& os, const UserData& user)
{
	os << user.username << "\t" << user.sockfd << "\t" << user.addr;
}