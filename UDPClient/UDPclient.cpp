////	Client side implementation of UDP client-server model 
#include <stdio.h>
#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <map>
#include <chrono>
#include <thread>

#pragma comment (lib, "ws2_32.lib")

void Shutdown(SOCKET sock)
{
	closesocket(sock);
	WSACleanup();
	exit(0);
}

int main()
{
	WSAData data;
	const WORD version = MAKEWORD(2, 2);

	const auto wsaResult = WSAStartup(version, &data);
	if (wsaResult != 0)
	{
		printf("Couldn't start WinSock: %i", wsaResult);
		return -1;
	}

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(54000);
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	bool hold = false;
	int lastIndex = 0;
	char acknowledgeBuffer[32];
	std::map<int, std::string> packetsToSend;

	while (true)
	{
		if (hold)
		{
			//Possibly Might Not work.....
			//std::this_thread::sleep_for(std::chrono::milliseconds(100));

			//Freeze input until acknowledgement from the server is received.
			const int bytesIn = recvfrom(sock, acknowledgeBuffer, 32, 0, nullptr, nullptr);
			if (bytesIn == SOCKET_ERROR)
			{
				printf("Acknowledgement did not work: %i\n", WSAGetLastError());
				return -3; //Might Change
			}

			std::string acknowledgedPacketIndex = acknowledgeBuffer;
			packetsToSend.erase(std::stoi(acknowledgedPacketIndex.substr(0, bytesIn)));

			//Trigger Once All Packets Have been Send
			if (packetsToSend.empty()) hold = false;
		}

		if (hold) continue;

		//Take in multiple inputs(until command EXIT or SEND is entered)
		std::string contentBuffer;
		while (true)
		{
			std::getline(std::cin, contentBuffer);

			//Send all input to the server at once, when SEND command is entered
			if (contentBuffer == "send")
			{
				for (auto i = packetsToSend.begin()->first; i <= packetsToSend.rbegin()->first; i++) {
					auto packet = std::to_string(i);
					packet += '^';
					packet += packetsToSend[i];
					
					const auto result = sendto(sock, packet.c_str(), packet.size(), 0, reinterpret_cast<sockaddr*>(&server), sizeof(server));
					if (result == SOCKET_ERROR)
					{
						printf("Sending did not work: %i\n", WSAGetLastError());
						return -2;
					}
					lastIndex++;
				}
				hold = true;
				break;
			}

			//Exit when EXIT command is entered
			if (contentBuffer == "exit")
			{
				const auto result = sendto(sock, "exit", strlen("exit"), 0, reinterpret_cast<sockaddr*>(&server), sizeof(server));
				if (result == SOCKET_ERROR)
				{
					printf("Sending did not work: %i\n", WSAGetLastError());
					return -2;
				}
				Shutdown(sock); 
			}

			//Add a sequence to each input(a number or timestamp which indicates the message's order)
			if (packetsToSend.empty())
				packetsToSend.emplace(std::make_pair(lastIndex, contentBuffer));
			else
				packetsToSend.emplace(std::make_pair(packetsToSend.rbegin()->first + 1, contentBuffer));
		}
	}
}
