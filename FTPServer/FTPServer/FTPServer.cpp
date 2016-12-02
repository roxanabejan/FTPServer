// FTPServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
#pragma comment(lib, "Wininet")

HINTERNET hOpenHandle, hConnectHandle, hFind;
vector<WIN32_FIND_DATA> vec;

bool list(vector<WIN32_FIND_DATA>& vec)
{
	if (!hConnectHandle)
		return false;

	WIN32_FIND_DATA fileInfo;

	hFind = FtpFindFirstFile(hConnectHandle, TEXT("*.*"), &fileInfo, 0, 0);
	if (hFind == NULL)
		return false;
	vec.push_back(fileInfo);

	while (InternetFindNextFile(hFind, &fileInfo) == TRUE)
	{
		vec.push_back(fileInfo);
	}

	InternetCloseHandle(hFind);

	return true;
}

string getFileExtention(wchar_t* fileName) {
	wstring ws(fileName);
	string str(ws.begin(), ws.end());
	return str.substr(str.find_last_of(".") + 1);
}

void printVector(vector<WIN32_FIND_DATA>& vec) { //print and save text files
	wchar_t newPath[256];

	for (vector<WIN32_FIND_DATA>::iterator it = vec.begin(); it != vec.end(); ++it) {
		swprintf(newPath, L"C:\\Users\\Roxana\\Documents\\GitHub\\FTPServer\\FTPServer\\FTPServer\\%ws", it->cFileName);
		printf("%ws %s\n", it->cFileName, getFileExtention(it->cFileName).c_str());

		if(getFileExtention(it->cFileName) == "txt")
			FtpGetFile(hConnectHandle, it->cFileName, newPath , TRUE, FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_UNKNOWN, 0);
	}	
}

void readFiles(vector<WIN32_FIND_DATA>& vec) {
	char file[256] = "file.txt";
	for (vector<WIN32_FIND_DATA>::iterator it = vec.begin(); it != vec.end(); ++it) {

		sprintf(file, "%ws", it->cFileName);
		ifstream infile(file);	
		string line;
		while (std::getline(infile, line))
		{
			printf("%s\n", line.c_str());
		}
		infile.close();
		printf("\n");
	}
}

int main(int argc, char* argv[])
{
	wchar_t serverAddress[256], username[256], password[256];
	if (argc < 4) {
		cerr << "Usage: <SERVER ADDRESS> <USERNAME> <PASSWORD>\n";
	}
	else {
		swprintf(serverAddress, L"%hs", argv[1]);
		swprintf(username, L"%hs", argv[2]);
		swprintf(password, L"%hs", argv[3]);
		
		printf("Server Address: %ws, username: %ws , password: %ws\n", serverAddress, username, password);
	}

	hOpenHandle = InternetOpen(TEXT("FTPConnection"),
							   INTERNET_OPEN_TYPE_PRECONFIG,
							   NULL, NULL, 0);
	if (hOpenHandle == NULL) {
		return printf("Library initialization failed %li.\n", GetLastError());
	}

	hConnectHandle = InternetConnect(hOpenHandle,
								     serverAddress, 
									 40056,
									 username,
									 password,
									 INTERNET_SERVICE_FTP,
									 INTERNET_FLAG_PASSIVE,
									 NULL);
	if (hConnectHandle == NULL)
		return printf("Connection to update server failed: %li.\n", GetLastError());

	if (list(vec) == false)
		return 1;
	printVector(vec);
	readFiles(vec);

	InternetCloseHandle(hConnectHandle);
	InternetCloseHandle(hOpenHandle);
    return 0;
}

