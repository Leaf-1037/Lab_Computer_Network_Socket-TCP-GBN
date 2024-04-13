#pragma once
#include "winsock2.h"
#include "Config.h"
#include <stdio.h>
#include <iostream>

const int MAX_STR_LENGTH = 25;

using namespace std;

#pragma comment(lib,"ws2_32.lib")

string config_path;

int flag = 0;

int ReadMyFile(string filename, Request& q)
{
	ifstream File(filename, ios::binary);
	if (!File) {
		cerr << "open error!" << endl;
		return 0;
	}
	if (!flag&&filename == config_path+"yzzx001.jpg") {
		flag = 1;
		cerr << "Invalid open!" << endl;
		return 0;
	}
	File.seekg(0, ios::end);
	size_t size = File.tellg();
	char* buf = new char[(size + 1)];
	memset(buf, '\0', (size + 1));
	File.seekg(0, ios::beg);
	File.read(buf, size);
	int j = strlen(buf);
	File.close();
	string s(buf, buf + size);
	q.body = s;
	return 1;
}

void main(){
	WSADATA wsaData;
	fd_set rfds;				//用于检查socket是否有数据到来的的文件描述符，用于socket非阻塞模式下等待网络事件通知（有数据到来）
	fd_set wfds;				//用于检查socket是否可以发送的文件描述符，用于socket非阻塞模式下等待网络事件通知（可以发送数据）
	bool first_connetion = true;

	int nRc = WSAStartup(0x0202,&wsaData);

	if(nRc){
		printf("Winsock  startup failed with error!\n");
	}

	if(wsaData.wVersion != 0x0202){
		printf("Winsock version is not correct!\n");
	}

	printf("Winsock  startup Ok!\n");


	SOCKET srvSocket;
	sockaddr_in addr,clientAddr;
	SOCKET sessionSocket;
	int addrLen;
	//create socket
	srvSocket = socket(AF_INET,SOCK_STREAM,0);
	if(srvSocket != INVALID_SOCKET)
		printf("Socket create Ok!\n\n");
	else {
		printf("Socket create FAIL!\n\n");
		WSAGetLastError();
	}

	//enter address
	//int srv_port;
	//char srv_ip[MAX_STR_LENGTH], file_addr[MAX_STR_LENGTH];// 监听地址和主目录
	//printf("Enter server_port: ");
	//scanf("%d", &srv_port);
	//printf("Enter server_IP: ");
	//scanf("%s", srv_ip);
	//printf("Enter file_address: ");
	//scanf("%s", file_addr);
	//puts("\n");


	rr::RrConfig config;
	bool ret = config.ReadConfig("Config.ini");
	if (ret == false) {
		printf("ReadConfig is Error,Cfg=%s", "config.ini");
		return;
	}

	std::string srv_ip = config.ReadString("Lab_1", "ip", "");
	int srv_port = config.ReadInt("Lab_1", "port", 0);
	std::string file_addr = config.ReadString("Lab_1", "path", "");
	config_path = file_addr;
	std::cout << "HostIP=" << srv_ip << std::endl;
	std::cout << "Port=" << srv_port << std::endl;
	std::cout << "FileAddr=" << file_addr << std::endl;
	


	//set port and ip
	addr.sin_family = AF_INET;
	addr.sin_port = htons(srv_port);
	//addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_addr.S_un.S_addr = inet_addr(srv_ip.c_str());


	//binding
	int rtn = bind(srvSocket,(LPSOCKADDR)&addr,sizeof(addr));
	if(rtn != SOCKET_ERROR)
		printf("Socket bind Ok!\n");
	else {
		printf("Socket bind FAILED!\n");
		WSAGetLastError();
	}
	//listen
	rtn = listen(srvSocket,5);
	if(rtn != SOCKET_ERROR)
		printf("Socket listen Ok!\n");
	else {
		printf("Socket listen FAILED!\n");
		WSAGetLastError();
	}
	
	clientAddr.sin_family =AF_INET;
	addrLen = sizeof(clientAddr);
	char recvBuf[4096];

	u_long blockMode = 1;//将srvSock设为非阻塞模式以监听客户连接请求

	if ((rtn = ioctlsocket(srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
		cout << "ioctlsocket() failed with error!\n";
		return;
	}
	cout << "ioctlsocket() for server socket ok!	Waiting for client connection and data\n";

	//清空read,write描述符，对rfds和wfds进行了初始化，必须用FD_ZERO先清空，下面才能FD_SET
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	//设置等待客户连接请求
	FD_SET(srvSocket, &rfds);

	while(true){
		//清空read,write描述符
		flag = 0;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);

		//设置等待客户连接请求
		FD_SET(srvSocket, &rfds);

		if (!first_connetion) {
			//设置等待会话SOKCET可接受数据或可发送数据
			FD_SET(sessionSocket, &rfds);
			FD_SET(sessionSocket, &wfds);
		}
		
		//开始等待
		//int nTotal = select(0, &rfds, &wfds, NULL, NULL);
		int nTotal = select(0, &rfds, &wfds, NULL, NULL);
		//printf("nTotal=%d\n", nTotal);
		//如果srvSock收到连接请求，接受客户连接请求
		if (FD_ISSET(srvSocket, &rfds)) {
			nTotal--;

			//产生会话SOCKET
			sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
			if (sessionSocket != INVALID_SOCKET)
				printf("Socket listen one client request!\n");

			//设置等待会话SOKCET可接受数据或可发送数据
			FD_SET(sessionSocket, &rfds);
			FD_SET(sessionSocket, &wfds);

			first_connetion = false;

		}
		//printf("nTotal=%d\n", nTotal);
		//检查会话SOCKET是否有数据到来
		if (nTotal > 0) {
			//如果会话SOCKET有数据到来，则接受客户的数据
			if (FD_ISSET(sessionSocket, &rfds)) {
				//receiving data from client
				memset(recvBuf, '\0', 4096);
				rtn = recv(sessionSocket, recvBuf, 256, 0);
				printf("rtn=%d\n\n", rtn);
				if (rtn > 0) {
					printf("Received %d bytes from client: %s\n", rtn, recvBuf);
					stringstream ss(recvBuf);
					Request rqst = rqst.Dealrqst(ss);
					//rqst.url=
					cout << "URL=" << rqst.url << endl;

					if (ReadMyFile(file_addr + rqst.url, rqst))
					{
						string rsbd = rqst.ResponseString(rqst, 200);
						int wtn = send(sessionSocket, rsbd.c_str(), rsbd.length(), 0);
						if (wtn == SOCKET_ERROR) {
							cout << "Send Failed!!" << endl;
							continue;
						}
						wtn = send(sessionSocket, rqst.body.c_str(), rqst.body.length(), 0);
						if (wtn == SOCKET_ERROR) {
							cout << "Send Failed!!" << endl;
						}
						cout << "Send success!" << endl;
						cout << "ClientAddr=" << inet_ntoa(clientAddr.sin_addr) << endl;
						cout << "ClientPort=" << clientAddr.sin_port << endl;
					}
					else
					{
						cout << "ClientVersion=" << rqst.version << endl;
						if (flag)cout <<"RQST##" << rqst.url << endl;
						if (flag) rqst.url = "403.html";
						if (flag) ReadMyFile(file_addr + "404.html", rqst);
						else ReadMyFile(file_addr + "404.html", rqst);

						string rsbd = flag? rqst.ResponseString(rqst, 404) : rqst.ResponseString(rqst, 404);
						int wtn = send(sessionSocket, rsbd.c_str(), rsbd.length(), 0);
						if (wtn == SOCKET_ERROR) {
							cout << "Send Failed!!" << endl;
						}
						wtn = send(sessionSocket, rqst.body.c_str(), rqst.body.length(), 0);
						if (wtn == SOCKET_ERROR) {
							cout << "Send Failed!!" << endl;
						}
						cout << "Send success!" << endl;
					}
				}
				else {
					printf("Client leaving ...\n");
					closesocket(sessionSocket);  //既然client离开了，就关闭sessionSocket
				}

			}
		}	
	}

}
