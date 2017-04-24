#include "CClient.h"

#include <iostream>
#include <string.h>
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <vector>

using namespace std;

vector <string> portList {"/dev/pts/5", "/dev/pts/3", "/dev/pts/2", "/dev/pts/4"};
//vector <string> portList {"/dev/ttyS0", "/dev/ttyS1", "/dev/ttyS2", "/dev/ttyS3"};


CClient::CClient(string filePath, int protocol)
{
	if(!isFileExist(filePath))
		throw ENOENT;
	this->filePath = filePath;
	
	switch(protocol){
		case INTRFCE_COM:
		default:
			Interface = new CComInterface;
	}
}


bool CClient::isFileExist(string filePath)
{
	fileHandler = open(filePath.c_str(), O_RDONLY);
	if(fileHandler < 0){
		cerr << "Failed to open file.";
		return false;
	}
	return true;
}


int CClient::Start()
{
	if(FindServer())
		portHandler = Interface->ReturnPort();
	else
		return -1;
	
	SendReady();
	if(!WaitReady())
		return ENOMSG;
	
	return SendData();
}


int CClient::FindServer()
{
	for(unsigned int index = 0; index < portList.size(); index++){
		if(Interface->OpenPort(portList[index])){
			if(TryHandshake())
				return 1;
		}
	}
	
	return 0;
}


int CClient::TryHandshake()
{
	int size = sizeof(COM_HNDSHAKE);
	char buf[size];
		
	Write(COM_HNDSHAKE, size);

	for(int i = 0; i < TIMEOUT; i++){
		cout << "Scaning\n";
		if(Read(buf, size))
			break;
	}

	errno = 0;
	
	if(!strcmp(buf, COM_HNDSHAKE))
		return 1;

	return 0;
}


int CClient::SendReady()
{
	cout << "SendReady " << portHandler << endl;
	
	int len = Write(COM_READY, sizeof(COM_READY));
    if (len != sizeof(COM_READY)) {
		cerr << "Writing failed\n";
    }
	
	return 1;
}


int CClient::WaitReady()
{
	int len;
	char buf[sizeof(COM_READY)];
		
	for(int i = 0; i < TIMEOUT; i++){
		cout << "Waiting ready signal\n";
		len = Read(buf, sizeof(buf));
		
        if (len > 0)
			break;
	}
	
	if(!strcmp(buf, COM_READY)){
		cout << "Ready to send data\n";
		return 1;
	}

	cerr << "Server found, but not answering\n";
	
	return 0;
}


int CClient::SendData()
{
	char name[255];
	char buf[sizeof(int) + 1];
	int strLen = FileSize();
	int len = 0;
	
	sprintf(buf, "%i", strLen);
	
	cout << "Sending " << strLen << " bytes\n";
	
	len = Write(buf, sizeof(buf));
	cout << "Sent " << len << " bytes as size\n";
	
	len = 0;
	while(ReadFile(name, 1) > 0){
		len += Write(name, 1);
	}
	
	cout << "Sent " << len << " bytes of data\n";

	return 1;
}


int CClient::Write(const void *buf, size_t nbyte)
{
	return Interface->doWrite(buf, nbyte);
}


int CClient::Read(void *buf, size_t nbyte)
{
	return Interface->doRead(buf, nbyte);
}


int CClient::WriteFile(const void *buf, size_t nbyte)
{
	return write(fileHandler, buf, nbyte);
}


int CClient::ReadFile(void *buf, size_t nbyte)
{
	return read(fileHandler, buf, nbyte);
}


int CClient::FileSize()
{
	int strLen = 0;
	
	cout << "Checking input file size.\n";
	
	char rd;
	while(read(fileHandler, &rd, 1) > 0){
		strLen++;
	}
	
	lseek(fileHandler, 0, SEEK_SET);
	
	return strLen;
}


CClient::~CClient()
{
	if(fileHandler != -1) {
		cout << "Closing file " << fileHandler << "...\n";
		close (fileHandler);
	}
	
	if(Interface != nullptr){
		delete Interface;
	}
	
	cout << "Done\n";
}
