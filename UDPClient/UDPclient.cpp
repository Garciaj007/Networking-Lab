// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <WS2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

const u_short PORT = 54000;

// Driver code 
int main(int argc, char* argv[]) {
	// Start Winsock
	WSAData data;
	WORD version = MAKEWORD(2, 2);
	int wResult = WSAStartup(version, &data);
	if (wResult != 0)
	{
		printf("Cannot start winsock!\n");
		return 1;
	}

	//Create a hint structure for the server
	sockaddr_in serverHint;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(PORT);
	inet_pton(AF_INET, "127.0.0.1", &serverHint.sin_addr);

	//Create Socket
	SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);

	//Send Message
	auto s = std::string(argv[1]);

	int sendResult = sendto(out, s.c_str(), s.size() + 1, 0, (sockaddr*)&serverHint, sizeof(serverHint));
	if (sendResult == SOCKET_ERROR)
	{
		printf("Could Not Send To Server! %i\n", WSAGetLastError());
	}

	//Close the socket
	closesocket(out);

	//Close Down winsock
	WSACleanup();
}