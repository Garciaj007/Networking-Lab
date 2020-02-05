// Server side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <WS2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

const u_short PORT = 54000;

// Driver code 
int main() {
	// Start Winsock
	WSAData data;
	WORD version = MAKEWORD(2, 2);
	int wResult = WSAStartup(version, &data);
	if (wResult != 0)
	{
		printf("Cannot start winsock!\n");
		return 1;
	}

	// Bind socket to IP
	SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(PORT);

	if (bind(in, (sockaddr*)& serverHint, sizeof(serverHint)) == SOCKET_ERROR)
	{
		printf("Cant bind socket! %i\n", WSAGetLastError());
		closesocket(in);
		WSACleanup();
		return 1;
	}

	sockaddr_in clientHint;
	int clientLen = sizeof(clientHint);
	ZeroMemory(&clientHint, clientLen);

	char buffer[1024];

	//Loop
	while (true)
	{
		ZeroMemory(buffer, 1024);

		// Wait for message
		int bytesIn = recvfrom(in, buffer, 1024, 0, (sockaddr*)&clientHint, &clientLen);
		if (bytesIn == SOCKET_ERROR)
		{
			printf("Error recieving from client! %i\n", WSAGetLastError());
			continue;
		}

		// Display message and Client Info
		char clientIP[256];
		ZeroMemory(clientIP, 256);

		inet_ntop(AF_INET, &clientHint.sin_addr, clientIP, 256);

		printf("[IP: %s] %s\n", clientIP, buffer);
	}
	//Close Socket
	closesocket(in);

	//Shutdown Winsock
	WSACleanup();

	return 0;
}