////	Server side implementation of UDP client-server model 
#include <stdio.h>
#include <WS2tcpip.h>
#include <map>
#include <string>
#pragma comment (lib, "ws2_32.lib")

//Acknowledge receipt of input

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

	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(54000);

	if (bind(sock, reinterpret_cast<sockaddr*>(&serverHint), sizeof(serverHint)) == SOCKET_ERROR)
	{
		printf("Socket bind failed! %i\n", WSAGetLastError());
		return -1;
	}

	sockaddr_in client;
	int clientLength = sizeof(client);
	std::map<int, std::string> packets;

	char buffer[1024];

	while (true) 
	{
		ZeroMemory(&client, clientLength);
		ZeroMemory(buffer, 1024);

		int recieveResult = recvfrom(sock, buffer, 1024,0, reinterpret_cast<sockaddr*>(&client), &clientLength);
		if (recieveResult == SOCKET_ERROR)
		{
			printf("Error recieving from the client %i\n", WSAGetLastError());
			continue;
		}

		char clientIP[256];
		ZeroMemory(clientIP, 256);

		inet_ntop(AF_INET, &client.sin_addr, clientIP, 256);
		printf("Message Recieved From [%s] %s\n", clientIP, buffer);

		auto content = std::string(buffer);
		
		if (content == "exit") break;

		std::string id;
		int position = 0; 
		for (int i = 0; i < 10; i++)
		{
			position++;
			if (buffer[i] == '^') break;
			id += buffer[i];
		}

		if (packets.find(std::atoi(id.c_str())) != packets.end()) continue;

		content = content.substr(position, recieveResult - 1);
		
		printf("Recieved ID: %i\n\tMessage: %s\n", std::atoi(id.c_str()), content.c_str());

		if (content == "exit") break;

		packets.emplace(std::make_pair(std::stoi(id), content));

		//Acknowledge receipt of input [Return the id of packets recieved]
		auto result = sendto(sock, id.c_str(), 256, 0, reinterpret_cast<sockaddr*>(&client), sizeof(clientLength));
		if (result == SOCKET_ERROR)
		{
			printf("Acknowledge did not work: %i\n", WSAGetLastError());
			//return -2;
		}
	}

	//Display all input received from the client in the correct sequence.
	printf("Ending Session...\nPrinting all packets recieved:\n");
	for (auto packet : packets)
	{
		printf("\t%i : %s\n", packet.first, packet.second.c_str());
	}

	closesocket(sock);
	WSACleanup();
}
