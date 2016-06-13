#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "SHLWAPI.lib")
#pragma comment(lib, "Mswsock.lib")

#include <WinSock2.h>
#include <iostream>

#include <windows.h>
#include <winsock.h>
#include <Mswsock.h>
#include <shlwapi.h>

#define ARRSIZE 255

void RecieveFile(SOCKET);

int main()
{
	char name[ARRSIZE];

	//WinSock startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(1, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		MessageBoxA(nullptr, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	SOCKADDR_IN addr; // Address for listening
	int addrlen = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111); // Port
	addr.sin_family = AF_INET; // IPv4 Socket

	SOCKET Connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Set Connection socket;
	while (connect(Connection, reinterpret_cast<SOCKADDR*>(&addr), addrlen) != 0)
	{
		int err = MessageBoxA(nullptr, "Failed to Connect, waiting for reconnect", "Error", MB_OKCANCEL | MB_ICONERROR);
		if (err == 2)
			return 0;
		Sleep(3000); // 3 sec timeout
		Connection = socket(AF_INET, SOCK_STREAM, NULL);
	}
	std::cout << "Connected!" << std::endl;
	//char MOTD[256];
	//recv(Connection, MOTD, sizeof(MOTD), NULL);
	//std::cout << "MOTD:" << MOTD << std::endl;

	std::cout << "Enter user's name: ";
	std::cin >> name;
	send(Connection, name, sizeof(name), NULL);

	if (CreateDirectory(name, nullptr) != 0)
	{
		std::cout << "User's folder was created." << std::endl;
	}

	RecieveFile(Connection);

	std::cout << "\n" << std::endl;
	system("pause");

	closesocket(Connection);
	WSACleanup();

	return 0;
}

void RecieveFile(SOCKET Sock)
{
	OVERLAPPED AsyncInfo;
	AsyncInfo.Offset = 0;
	AsyncInfo.OffsetHigh = 0;
	AsyncInfo.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	DWORD dwbr;

	char buffer[MAX_PATH];
	char szFileName[MAX_PATH];
	ZeroMemory(buffer, MAX_PATH);
	ZeroMemory(szFileName, MAX_PATH);

	recv(Sock, szFileName, sizeof(szFileName), NULL);
	std::cout << "File check: " << szFileName << std::endl;
	HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	while (1)
	{
		ReadFile(reinterpret_cast<HANDLE>(Sock), &buffer, sizeof(buffer), &dwbr, &AsyncInfo);
		if (dwbr == 0)
			break;
		GetOverlappedResult(reinterpret_cast<HANDLE>(Sock), &AsyncInfo, &dwbr, TRUE);
		WriteFile(hFile, buffer, dwbr, &dwbr, nullptr);
	}
	CloseHandle(hFile);
}
/*
void RecieveFile(char szIP[], int port)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);
	SOCKET Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	in_addr iaHost;
	iaHost.s_addr = inet_addr(szIP);
	LPHOSTENT lphostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
	SOCKADDR_IN serverInfo = { AF_INET ,htons(port) ,*((LPIN_ADDR)*lphostEntry->h_addr_list) };
	connect(Sock, (LPSOCKADDR)&serverInfo, sizeof(struct sockaddr));
	OVERLAPPED AsyncInfo;
	AsyncInfo.Offset = 0;
	AsyncInfo.OffsetHigh = 0;
	AsyncInfo.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	DWORD dwbr;
	char buffer[MAX_PATH];
	char szFileName[MAX_PATH];
	recv(Sock, szFileName, MAX_PATH, 0);
	HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	while (1)
	{
		ReadFile((HANDLE)Sock, &buffer, MAX_PATH, &dwbr, &AsyncInfo);
		GetOverlappedResult((HANDLE)Sock, &AsyncInfo, &dwbr, TRUE);
		if (dwbr == 0) break;
		WriteFile(hFile, buffer, dwbr, &dwbr, NULL);
	}
	closesocket(Sock);
	WSACleanup();
	CloseHandle(hFile);
}*/