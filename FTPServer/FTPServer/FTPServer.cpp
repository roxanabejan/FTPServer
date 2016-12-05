// FTPServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <wininet.h>
#include <atlstr.h>
#include <regex>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
#pragma comment(lib, "Wininet")

HINTERNET hOpenHandle, hConnectHandle, hFind, hIntOpen;
vector<WIN32_FIND_DATA> vec;
vector<string> links;

bool list(vector<WIN32_FIND_DATA>& vec)
{
	WIN32_FIND_DATA fileInfo;
	if (!hConnectHandle)
		return false;

	hFind = FtpFindFirstFile(hConnectHandle, TEXT("*.*"), &fileInfo, 0, 0);
	if (hFind == NULL)
		return false;
	vec.push_back(fileInfo);

	while (InternetFindNextFile(hFind, &fileInfo) == TRUE)
	{
		if (!(fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			vec.push_back(fileInfo);
	}
	InternetCloseHandle(hFind);
	return true;
}

string getFileExtention(wchar_t* fileName) {
	wstring ws(fileName);
	string str(ws.begin(), ws.end());
	return str.substr(str.find_last_of("."));
}

void printLinks(vector<string> links) { 
	
	for (vector<string>::iterator it = links.begin(); it != links.end(); ++it) {
		printf("%hs\n", (*it).c_str());
	}
}
void printVector(vector<WIN32_FIND_DATA> vec) { //print and save text files
	wchar_t newPath[256];

	for (vector<WIN32_FIND_DATA>::iterator it = vec.begin(); it != vec.end(); ++it) {
		swprintf(newPath, L"C:\\Users\\Roxana\\Documents\\GitHub\\FTPServer\\FTPServer\\FTPServer\\%ws", it->cFileName);
		printf("%ws\n", it->cFileName);

		if(getFileExtention(it->cFileName) == ".txt")
			if(!FtpGetFile(hConnectHandle, it->cFileName, newPath , false , FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_ASCII, 0))
				printf("FTPGetFile failed.\n");
	}	
}

bool readFiles(vector<WIN32_FIND_DATA> vec, vector<string>& links) {
	char file[256];
	for (vector<WIN32_FIND_DATA>::iterator it = vec.begin(); it != vec.end(); ++it) {

		sprintf(file, "%ws", it->cFileName);
		ifstream infile(file);	
		string line;
		while (getline(infile, line))
		{
			if (!line.compare(0, 4, "http") && line.substr(line.find_last_of(".") + 1) == "exe") {
				links.push_back(line);
				printf("%s\n", line.c_str());
			}
		}
		infile.close();
	}
	return 0;
}

void get_links_from_file(string file_string, vector<string>* links) {
	regex re("http:.*\\.exe");
	sregex_iterator next(file_string.begin(), file_string.end(), re);
	sregex_iterator end_re;

	while (next != end_re) {
		smatch match = *next;
		links->push_back(match.str());
		next++;
	}
}

void execute_file(wchar_t* file_path) {
	char filepath[1000];
	int i;

	sprintf(filepath, "%ws\n", file_path);
	printf("Checking if processor is available...");
	if (system(NULL))
		puts("Ok");
	else
		exit(1);
	printf("Executing app...\n");
	i = system(filepath);
	printf("The value returned was: %d.\n", i);

	getchar();
}

void download(vector<string> links) {
	for (vector<string>::iterator iter = links.begin(), end = links.end(); iter != end; ++iter) {
		string url = *iter;
		wchar_t w_url[1000], w_dest_file[1000];
		swprintf(w_url, L"%hs", url.c_str());
		HINTERNET internet_open_url = InternetOpenUrl(hIntOpen,
										    		  w_url,
													  NULL, NULL, NULL, NULL);

		string filename = url.substr(url.rfind("/") + 1, url.npos);
		swprintf(w_dest_file, L"D:\\College\\%hs", filename.c_str());
		wprintf(L"%ws\n", w_dest_file);

		HANDLE hFile = CreateFile(w_dest_file, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		char Buffer[1024];
		DWORD dwRead = 0;
		while (InternetReadFile(internet_open_url, Buffer, sizeof(Buffer), &dwRead) == TRUE)
		{
			if (dwRead == 0)
				break;
			DWORD dwWrite = 0;
			WriteFile(hFile, Buffer, dwRead, &dwWrite, NULL);
		}
		CloseHandle(hFile);
		InternetCloseHandle(internet_open_url);

		execute_file(w_dest_file);
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
		return printf("FTP Library initialization failed %li.\n", GetLastError());
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

	hIntOpen = InternetOpen(TEXT("HTTPServer"), LOCAL_INTERNET_ACCESS, NULL, 0, 0);
	if (hIntOpen == NULL)
		return printf("HTTP Library initialization failed %li.\n", GetLastError());

	if (list(vec) == false)
		return 1;
	printVector(vec);

	if (readFiles(vec, links) == 1) {
		return 1;
	}

	download(links);

	InternetCloseHandle(hConnectHandle);
	InternetCloseHandle(hOpenHandle);
    return 0;
}

