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
const PCSTR DEFAULT_HOST = "localhost";
const int DEFAULT_BUFFER_SEND_LENGTH = 80;
const int DEFAULT_BUFFER_RECIEVE_LENGTH = 512;

void Close(SOCKET s)
{
	closesocket(s);
	WSACleanup();
	exit(0);
}

int main()
{
	WSADATA wsaData;

	struct addrinfo* result = nullptr, * ptr = nullptr, hints;

	SOCKET connectSocket = INVALID_SOCKET;

	char sendBuffer[DEFAULT_BUFFER_SEND_LENGTH];
	char recieveBuffer[DEFAULT_BUFFER_RECIEVE_LENGTH];

	auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("init failed with error code %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(DEFAULT_HOST, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddres failed with error code %d\n", iResult);
		WSACleanup();
		return 1;
	}

	for (ptr = result; ptr != nullptr; ptr->ai_next)
	{
		connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (connectSocket == INVALID_SOCKET)
		{
			printf("socket failed with error code %d\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}

		iResult = connect(connectSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
		if (iResult == SOCKET_ERROR)
		{
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	freeaddrinfo(result);
	if (connectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server \n");
		WSACleanup();
		return 1;
	}

	printf("tcp_net_lab > Type a command to send to server, or 'quit' to disconnect...\n");

	//Constantly Running...
	while (true)
	{
		printf("\ntcp_net_lab > ");
		fgets(sendBuffer, DEFAULT_BUFFER_SEND_LENGTH, stdin);
		sendBuffer[79] = '\0';
		iResult = send(connectSocket, sendBuffer, static_cast<int>(strlen(sendBuffer)), 0);

		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error code %d\n", WSAGetLastError());
			break;
		}

		do
		{
			iResult = recv(connectSocket, recieveBuffer, DEFAULT_BUFFER_RECIEVE_LENGTH, 0);
			if (iResult > 0)
			{
				auto recieveStr = std::string(recieveBuffer);
				recieveStr = recieveStr.substr(0, iResult);
				printf(recieveStr.c_str());
				break;
			}
			else if (iResult == 0) 
			{
				printf("Connection Severed");
				Close(connectSocket);
			}
			else
			{
				printf("recv failed with error code: %d\n", iResult);
				Close(connectSocket);
			}
		} while (iResult > 0);
	}

	Close(connectSocket);

}