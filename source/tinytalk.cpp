#include "tinytalk.h"

TinyTalk::TinyTalk(int port)
{
	user = new UserData;

	this->port = port;
	this->sockfd = -1;
}

TinyTalk::~TinyTalk()
{
	if (user != nullptr)
	{
		delete user;
	}
	if (sockfd != -1)
	{
		close(sockfd);
		sockfd = -1;
	}
}

bool TinyTalk::createTCPServer()
{
	// create TCP socket server: SOCK_STREAM
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		std::cout << "ERROR: Failed to create a socket." << std::endl;
		return false;
	}
	// 启用端口复用 SO_REUSEADDR
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	// 设置服务器协议，端口和地址
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	// 将 sockfd 绑定到服务器地址
	if (bind(sockfd, (struct sockaddr*)&sa, sizeof(sa)) == -1)
	{
		std::cout << "Failed to bind sockfd to the server address." << std::endl;
		return false;
	}
	if (listen(sockfd, 511) == -1)
	{
		std::cout << "ERROR: Failed to listen on sockfd." << std::endl;
		return false;
	}
	return true;
}

int TinyTalk::acceptUser(struct sockaddr_in &sa)
{
	int userSockfd;
	while (1)
	{
		socklen_t slen = sizeof(sa);
		userSockfd = accept(sockfd, (struct sockaddr*)&sa, &slen);
		if (userSockfd == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else
			{
				return -1;
			}
		}
		break;
	}
	return userSockfd;
}

void TinyTalk::welcomeMSG(int fd)
{
	std::string msg = "Welcome to TinyTalk!\n"
		"Use /exit to exit.\n"
		"    /name <name> to set your name.\n";
	write(fd, msg.c_str(), strlen(msg.c_str()));
	std::cout << "Successfully connected to a client (client fd = " << fd  << ")" << std::endl;
}

void TinyTalk::exitMSG(int fd)
{
	std::string msg = "TinyTalk:Bye-Bye!\n";
	write(fd, msg.c_str(), strlen(msg.c_str()));
	std::cout << "Server has exited" << std::endl;
}

void TinyTalk::addUser(const int fd, const std::string ip)
{
	user->setFD(fd);
	user->setName(std::to_string(fd));
	user->setAddr(ip);
}

void TinyTalk::run()
{
	// 欢迎界面
	std::cout << "Hello!\tWelcome to TinyTalk!" << std::endl;
	std::cout << "TinyTalk is running on port " << this->port << std::endl;

	/*
	*  创建 TCP 服务器
	*  1. 使用 socket() 函数创建 socket
	*  2. 使用 bind() 函数将该 socket 绑定到服务器地址上
	*  3. 使用 listen() 函数将该 socket 标记为被动套接字
	*/
	if (!this->createTCPServer())
	{
		std::cout << "ERROR: Failed to create TCP server." << std::endl;
		return;
	}

	// 等待客户端主动连接服务器
	struct sockaddr_in userAddr;
	memset(&userAddr, 0, sizeof(userAddr));
	int userSockfd = acceptUser(userAddr);
	if (userSockfd == -1)
	{
		std::cout << "ERROR: Failed to accept a connection on the socket." << std::endl;
		return;
	}

	// 给客户端发送欢迎消息
	welcomeMSG(userSockfd);
	// 保存客户端信息
	addUser(userSockfd, inet_ntoa(userAddr.sin_addr));

	while (1)
	{
		// 设置读取客户端的缓冲区
		char readbuf[512];
		memset(&readbuf, 0, sizeof(readbuf));
		// 读取客户端发送的信息
		int nread = read(user->getFD(), readbuf, sizeof(readbuf) - 1);
		if (nread <= 0)
		{
			std::cout << "User has disconnected!!!" << std::endl;
		}
		else
		{
			std::string message;
			int index = parseCommand(readbuf, nread - 1, message);
			if (!index) break;
			switch (index)
			{
			case -2: 
			{
				std::string errorInfo = "ERROR: Invalid command or incorrect command usage.\n";
				write(user->getFD(), errorInfo.c_str(), strlen(errorInfo.c_str()));
				break;
			}
			case -1: 
			{
				std::string errorInfo = "ERROR: Received message is invalid.\n";
				write(user->getFD(), errorInfo.c_str(), strlen(errorInfo.c_str()));
				break;
			}
			case 0: break;
			case 1: 
			{
				std::cout << user->getName() << "(" << user->getFD() << ":" << user->getAddr() << ")>>";
				std::cout << message << std::endl;
				break;
			}
			case 2: 
			{
				user->setName(message);
				std::cout << "$$$ client " << user->getFD() << " has set a new name: "
					<< user->getName() << " $$$" << std::endl;
				break;
			}
			default:
				break;
			}
		}
	}
	exitMSG(userSockfd);
}

int TinyTalk::parseCommand(const char* command, const int length, std::string &context)
{
	/*
	*  -1: 消息不是以\n结尾
	*  -2: 指令没有参数/不支持该指令
	*  0: 退出
	*  1: 发送
	*  2: 设置名称
	*/

	// 去除末尾的多余符号
	std::string commandStr(command);
	std::size_t found = (commandStr.find("\r\n") == std::string::npos)? commandStr.find("\n") : commandStr.find("\r\n");
	if (found == std::string::npos)
	{
		return -1;
	}
	commandStr = commandStr.substr(0, found);

	// 指令模式
	if (commandStr[0] == '/')
	{
		// 分割指令和内容
		std::size_t separator = commandStr.find(" ");
		if (separator == std::string::npos)
		{
			if (commandStr.find("exit") == 1)
			{
				return 0;
			}
			return -2;
		}
		std::string commandName = commandStr.substr(1, separator - 1);
		context = commandStr.substr(separator + 1, found);
		if (commandName == "name")
		{
			return 2;
		}
		else
		{
			return -2;
		}
	}
	// 发送模式
	context = commandStr;
	return 1;
}