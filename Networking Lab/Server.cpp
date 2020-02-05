#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <iphlpapi.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

const PCSTR DEFAULT_PORT = "27015";
const int DEFAULT_BUFFER_SEND_LENGTH = 512;
const int DEFAULT_BUFFER_RECIEVE_LENGTH = 80;

void Close(SOCKET s)
{
	closesocket(s);
	WSACleanup();
}

void Shutdown(SOCKET client, SOCKET listen)
{
	auto iResult = shutdown(client, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("clien shutdown failed with error code %d\n", WSAGetLastError());
		Close(listen);
		exit(1);
	}
	Close(client);
	exit(0);
}

int main()
{
	WSAData wsaData;
	struct addrinfo* result = nullptr, * ptr = nullptr, hints;

	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET clientSocket = INVALID_SOCKET;

	//char sendBuffer[DEFAULT_BUFFER_SEND_LENGTH];
	char recieveBuffer[DEFAULT_BUFFER_RECIEVE_LENGTH];

	std::string infoStr;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//initialize winsocket
	auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("init failed with error code %d\n", iResult);
		return 1;
	}

	iResult = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddres failed with error code %d\n", iResult);
		WSACleanup();
		return 1;
	}

	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error code %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(listenSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error code %d\n", WSAGetLastError());
		closesocket(listenSocket);
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("listen failed with error code %d\n", WSAGetLastError());
		Close(listenSocket);
		return 1;
	}

	clientSocket = accept(listenSocket, nullptr, nullptr);

	if (clientSocket == INVALID_SOCKET)
	{
		printf("accept failed with error code %d\n", WSAGetLastError());
		Close(listenSocket);
		return 1;
	}

	closesocket(listenSocket);

	int infoBufferPage = 0;
	bool prepareForInfo = false;
	do
	{
		iResult = recv(clientSocket, recieveBuffer, DEFAULT_BUFFER_RECIEVE_LENGTH, 0);
		if (iResult > 0)
		{
			printf("Bytes Recieved: %d\n", iResult);
			auto recieveStr = std::string(recieveBuffer);
			recieveStr = recieveStr.substr(0, iResult - 1);

			auto sendStr = std::string();
			do {
				if (recieveStr == "quit")
				{
					Shutdown(clientSocket, listenSocket);
					break;
				}
				else if (recieveStr == "help-prepare")
				{
					sendStr = "input - Appends information to the info string\n";
					sendStr += "flush - Clears info buffer.\n";
					sendStr += "recall - Gets a string of latest info.\n";
					sendStr += "suspend - Exits Prepare Mode.\n";
					break;
				}
				else if (recieveStr == "help")
				{
					sendStr = "quit - Terminates Session.\n";
					sendStr += "help - Shows Lists of Commands\n";
					sendStr += "prepare - Ready's server for additional commands.\n";
					sendStr += "Type 'help-prepare' for more info";
					break;
				}
				else if (prepareForInfo)
				{
					if (recieveStr.substr(0, 5) == "input")
					{
						infoStr += recieveStr.substr(6);
						sendStr = "Recieved.";
						break;
					}
					else if (recieveStr == "flush")
					{
						infoStr.clear();
						sendStr = "Cleared.";
						break;
					}
					else if (recieveStr == "recall")
					{
						sendStr = infoStr;
						break;
					}
					else if (recieveStr == "suspend")
					{
						prepareForInfo = false;
						sendStr = "Prepare Mode Disengaged";
						break;
					}
					else
					{
						sendStr = "Cmd not defined: ";
						sendStr += recieveStr;
						break;
					}
				}
				else if (recieveStr == "prepare")
				{
					prepareForInfo = true;
					sendStr = "Prepare Mode Engaged: Ready For Messages...";
					break;
				}

				if (recieveStr != "help-prepare" && recieveStr != "help" && recieveStr != "quit" && recieveStr != "prepare")
				{
					sendStr = "Cmd not defined: ";
					sendStr += recieveStr;
					break;
				}
			} while (true);

			auto iSendResult = send(clientSocket, sendStr.c_str(), DEFAULT_BUFFER_SEND_LENGTH, 0);
			if (iSendResult == SOCKET_ERROR)
			{
				printf("send failed with error code %d\n", WSAGetLastError());
				closesocket(listenSocket);
				WSACleanup();
				return 1;
			}

			sendStr.clear();
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
		{
			printf("Connection is severed\n");
		}
		else if (iResult < 0)
		{
			printf("recv failed with error code %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	Shutdown(clientSocket, listenSocket);
}