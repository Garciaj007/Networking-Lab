////	Client side implementation of UDP client-server model 
#include <stdio.h>
#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <map>

#pragma comment (lib, "ws2_32.lib")

int main()
{
	WSAData data;
	WORD version = MAKEWORD(2, 2);

	auto wsaResult = WSAStartup(version, &data);
	if (wsaResult != 0)
	{
		printf("Couldn't start WinSock: %i", wsaResult);
		return -1;
	}

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(54000);
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	int packetCounter = 0;
	bool hold = false;
	std::map<int, std::string> packetsToSend;

	char recieveBuffer[256];

	while (true)
	{
		int bytesIn = recvfrom(sock, recieveBuffer, 256, 0, nullptr, nullptr);
		if (bytesIn == SOCKET_ERROR)
		{

		}

		if (hold) continue;

		std::string contentBuffer;
		
		while (true)
		{
			std::cin >> contentBuffer;
			std::cout << contentBuffer << std::endl;

			if (contentBuffer == "send")
			{
				auto result = sendto(sock, packetsToSend[packetCounter].c_str(), packetsToSend[packetCounter].size(), 0, reinterpret_cast<sockaddr*>(&server), sizeof(server));
				if (result == SOCKET_ERROR)
				{
					printf("Sending did not work: %i\n", WSAGetLastError());
					return -2;
				}
				hold = true;
			}

			if (contentBuffer == "exit")
			{
				auto result = sendto(sock, "exit", strlen("exit"), 0, reinterpret_cast<sockaddr*>(&server), sizeof(server));
				if (result == SOCKET_ERROR)
				{
					printf("Sending did not work: %i\n", WSAGetLastError());
					return -2;
				}
			}
		}
	}

	closesocket(sock);
	WSACleanup();
}