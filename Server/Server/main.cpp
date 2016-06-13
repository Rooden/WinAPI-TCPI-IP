#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "SHLWAPI.lib")
#pragma comment(lib, "Mswsock.lib")

#include <WinSock2.h>
#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <Wincrypt.h>

#include <Mswsock.h>
#include <shlwapi.h>

#define ARRSIZE 255
#define BUFSIZE 1024
#define MD5LEN  16

DWORD  getMD5(LPCSTR filename);

void   findAllFiles(LPCSTR);
LPCSTR merge(char*, char*);
void SendFile(char*, int);

int main()
{
	char name[ARRSIZE];

	//WinSock startup
	WSAData wsaData;
	WORD	DllVersion = MAKEWORD(1, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		MessageBoxA(nullptr, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	SOCKADDR_IN addr; // Address for listening
	int addrlen = sizeof(addr);
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(1111); // Port
	addr.sin_family = AF_INET; // IPv4 Socket

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Socket for listening
	bind(sListen, reinterpret_cast<SOCKADDR*>(&addr), addrlen); // Bind the address to the socket
	listen(sListen, SOMAXCONN);

	SOCKET newConnection; // Socket to hold the client's connection
	newConnection = accept(sListen, reinterpret_cast<SOCKADDR*>(&addr), &addrlen); // Accept a new connection
	if (newConnection == 0)
	{
		std::cout << "Failed to accept the client's connection." << std::endl;
	}
	else
	{
		std::cout << "Client Connected!" << std::endl;
		//findAllFiles(merge("RooDen", "*.*"));
		//auto hash = getMD5("RooDen\\test.txt");
		//char MOTD[256] = "Welcome! This is Message of the Day.";
		//send(newConnection, MOTD, sizeof(MOTD), NULL);
		recv(newConnection, name, sizeof(name), NULL);
		std::cout << "Current user:" << name << std::endl;

		if (CreateDirectory(name, nullptr) != 0)
		{
			std::cout << "User's folder was created." << std::endl;
		}
		HANDLE hFile = CreateFile("RooDen\\test.exe",
			GENERIC_READ | GENERIC_WRITE,
			0,
			nullptr,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			nullptr);
		//char* szFileName = PathFindFileName("RooDen\\test.txt");
		send(newConnection, "RooDen\\test.exe", sizeof("RooDen\\test.exe"), 0);
		TransmitFile(newConnection, hFile, 0, 1024, nullptr, nullptr, TF_USE_KERNEL_APC);
		CloseHandle(hFile);
	}

	std::cout << "\n" << std::endl;
	system("pause");

	closesocket(newConnection);
	closesocket(sListen);
	WSACleanup();

	return 0;
}

DWORD getMD5(LPCSTR filename)
{
	DWORD		dwStatus = 0;
	BOOL		bResult = FALSE;
	HCRYPTPROV	hProv = 0;
	HCRYPTHASH	hHash = 0;
	HANDLE		hFile;
	BYTE		rgbFile[BUFSIZE];
	DWORD		cbRead = 0;
	BYTE		rgbHash[MD5LEN];
	DWORD		cbHash;
	CHAR		rgbDigits[] = "0123456789abcdef";
	// Logic to check usage goes here.

	hFile = CreateFile(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		nullptr);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		dwStatus = GetLastError();
		printf("Error opening file %s\nError: %d\n", filename, dwStatus);
		return dwStatus;
	}

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv,
		nullptr,
		nullptr,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT))
	{
		dwStatus = GetLastError();
		printf("CryptAcquireContext failed: %d\n", dwStatus);
		CloseHandle(hFile);
		return dwStatus;
	}

	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
	{
		dwStatus = GetLastError();
		printf("CryptAcquireContext failed: %d\n", dwStatus);
		CloseHandle(hFile);
		CryptReleaseContext(hProv, 0);
		return dwStatus;
	}

	while ((bResult = ReadFile(hFile, rgbFile, BUFSIZE, &cbRead, nullptr)))
	{
		if (0 == cbRead)
		{
			break;
		}

		if (!CryptHashData(hHash, rgbFile, cbRead, 0))
		{
			dwStatus = GetLastError();
			printf("CryptHashData failed: %d\n", dwStatus);
			CryptReleaseContext(hProv, 0);
			CryptDestroyHash(hHash);
			CloseHandle(hFile);
			return dwStatus;
		}
	}

	if (!bResult)
	{
		dwStatus = GetLastError();
		printf("ReadFile failed: %d\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		CloseHandle(hFile);
		return dwStatus;
	}

	cbHash = MD5LEN;
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
	{
		printf("MD5 hash of file %s is: ", filename);
		for (DWORD i = 0; i < cbHash; i++)
		{
			printf("%c%c", rgbDigits[rgbHash[i] >> 4],
				rgbDigits[rgbHash[i] & 0xf]);
		}
		printf("\n");
	}
	else
	{
		dwStatus = GetLastError();
		printf("CryptGetHashParam failed: %d\n", dwStatus);
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	CloseHandle(hFile);

	return dwStatus;
}

void findAllFiles(LPCSTR filepath)
{
	int				count = 0;
	WIN32_FIND_DATA FindData;
	HANDLE			Handle = FindFirstFileA(filepath, &FindData);

	while (FindNextFile(Handle, &FindData))
	{
		if (strcmp(FindData.cFileName, "..") != 0)
		{
			if (getMD5(merge("RooDen", FindData.cFileName)) == 0)
			{
				count++;
			}
		}
	}

	if (count == 0)
	{
		std::cout << "Nothing here!" << std::endl;
	}
	CloseHandle(Handle);
}

LPCSTR merge(char *str1, char *str2)
{
	char *result = new char[strlen(str1) + strlen(str2) + 3];

	strcpy(result, str1);
	strcat(result, "\\");
	strcat(result, str2);

	return static_cast<LPCSTR>(result);
}