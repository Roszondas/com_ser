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

vector <string> portList {"/dev/pts/5", "/dev/pts/2", "/dev/pts/3", "/dev/pts/4"};
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
	portHandler = Interface->FindServer();
	
	cout << "Start " << portHandler << endl;
	
	if(portHandler == -1) return ENXIO;
	
	int res = Transmit();
	
	cout << "Start exiting " << portHandler << endl;
	
	return res;
}


int CClient::Transmit()
{
	SendReady();
	if(WaitReady() == -1)
		return ENOMSG;
	
	return SendData();
}


int CClient::SendReady()
{
	int len = write(portHandler, COM_READY, sizeof(COM_READY));
    if (len != sizeof(COM_READY)) {
		cerr << "Writing failed\n";
    }
    tcdrain(portHandler);
	
	return 0;
}


int CClient::Write(const void *buf, size_t nbyte)
{
	return Interface->doWrite(buf, nbyte);
}


int CClient::WaitReady()
{
	char buf[sizeof(COM_READY)];
	int len;
	
	for(int i = 0; i < TIMEOUT; i++){
		cout << "Waiting ready signal\n";
		len = read(portHandler, buf, sizeof(buf));
		
        if (len > 0)
			break;
		else if (len < 0)
			cerr << "Ready status recieve error\n";
	}
	
	if(!strcmp(buf, COM_READY))
		cout << "Ready to send data\n";
	else {
		cerr << "Server found, but not answering\n";
		return -1;
	}


	return 0;
}


int CClient::SendData()
{
	char name[255];
	
	int strLen = 0;
	
	cout << "Checking input file size.\n";
	
	//int chk = 0;
	char rd;
	
	while(read(fileHandler, &rd, 1) > 0){
		//cout << rd;
		//cout << chk++ << " bytes checked\n";
		strLen++;
	}
	
	//cout << "lseek\n";
	lseek(fileHandler, 0, SEEK_SET);

	//cout << "buf\n";
	char buf[sizeof(int) + 1];
	
	//strLen += 1;
	//cout << "sprintf\n";
	
	sprintf(buf, "%i", strLen);
	
	cout << "Sending " << strLen << " bytes\n";
	
	int len = write(portHandler, buf, sizeof(buf));
	cout << "Sent " << len << " bytes as size\n";
	
	len = 0;
	while(read(fileHandler, name, 1) > 0){
		len += write(portHandler, name, 1);
	}
	cout << "Sent " << len << " bytes of data\n";
	tcdrain(portHandler);
	
	return 0;
}


CClient::~CClient()
{
	cout << "Close handlers\n";
	if(portHandler != -1){
		cout << "Closing port " << portHandler << "...\n";
		tcsetattr(portHandler, TCSANOW, &savedOptions);
		ioctl(portHandler, TIOCNXCL);
		close (portHandler);
	}
	
	if(fileHandler != -1) {
		cout << "Closing file " << fileHandler << "...\n";
		close (fileHandler);
	}
	
	if(Interface != nullptr)
		delete Interface;
	
	cout << "Done\n";
}
