#include "CServer.h"
#include <iostream>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>



CServer::CServer(vector <string> portAdresses, int protocol)
{
	cout << "CServer\n";
	
	for(unsigned int index = 0; index < portAdresses.size(); index++){
		CInterface *Interface;
		switch(protocol){
			case INTRFCE_COM:
			default:
				Interface = new CComInterface;
		}
		
		if(Interface->OpenPort(portAdresses[index])){
			channelData.push_back(ChannelState(Interface, WAITING));
		}
	}
	
	if(channelData.empty()){
		cerr << "There is no working port\n";
		throw ENXIO;
	}
	
	cout << "\n";
}


int CServer::Start()
{
	int errFlag = 1;

	while(errFlag){
		for(auto &hndlr : channelData){
			actualChannel = &hndlr;
			switch(hndlr.status){
				case READING:	errFlag = RecieveData();
				case WAITING:
				case HNDSHAKED:	
				default:		errFlag = CheckReady();
			}
		}
	}
	
	return errFlag;
}


int CServer::CheckReady()
{
	int size = sizeof(COM_HNDSHAKE);
	char buf[size];
	memset(buf, '\0', size);
	
	//cout << "Scan port " << actualChannel->handler << endl;

	int len = Read(buf, size);
	if(len < 0){
		cerr << "Handshake recieve error.\n";
	}

	//cerr << "Len " << len << " Get " << buf << " wait " << COM_HNDSHAKE "\n " << strcmp(buf, COM_HNDSHAKE) << endl;
	
	if(!strcmp(buf, COM_HNDSHAKE)){
		len = 0;
		
		cout << "Port " << actualChannel->portName << " recieved handshake. Waiting for confirmation.\n";
		
		len = Write(COM_HNDSHAKE, sizeof(buf));
		if (len < 0) {
			cerr << "Writing confirm ready failed\n";
			return 0;
		}
	
		len = 0;
		memset(buf, 0, sizeof(buf));
		
		for(int i = 0; i < TIMEOUT; i++){
			len = Read(buf, sizeof(buf));
			
			if (len > 0)
				break;
			else if (len < 0){
				cerr << "Confirmation recieve error\n";
				return 0;
			}
		}
		
		//cerr << "Len " << len << " Get " << buf << " wait " << COM_READY << endl;
		
		if(len > 0 && !strcmp(buf, COM_READY)){
			len = Write(COM_READY, sizeof(COM_READY));
			if (len < 0) {
				cerr << "Writing ready signal failed\n";
				return 0;
			}
			
			actualChannel->status = READING;
			cout << "Confirmation recieved. Ready to write data. Status " << actualChannel->status << endl;
		}
	}
	
	return 1;
}


int CServer::RecieveData()
{
	char buf[sizeof(int) + 1];
	memset(buf, 0, sizeof(buf));
	int err = 0;
	
	cout << "Waiting data " << actualChannel->handler << endl;
	
	int len = 0;
		
	while(len < sizeof(buf)){
		err = Read(buf, sizeof(buf));
		if (err < 0){
			cerr << "Recieving size error\n";
			return 0;
		}
		
		len += err;
	}
	
	cout << "buf = " << buf << endl;
	int msgSize = atoi(buf);
	cout << "Ready to recieve " << msgSize << " bytes\n";
	
	err = WriteData(msgSize);
	actualChannel->CloseFile();
	actualChannel->status = WAITING;
	
	cout << "Data recieved and saved\n\n";
	
	return err;
}


int CServer::WriteData(int size)
{
	//char *buf = new char[size];
	char buf;
	int err = 0;
	int len = 0;
	
	cout << "Reading data.\n";
	while(len < size){
		err = Read(&buf, 1);
		if (err < 0){
			cerr << "Recieving data error\n";
			return 0;
		}
		
		len += err;

		//cout << "Read " << len << " bytes\n";
		//cout << "Data: " << buf << endl;
	
		int wlen = WriteFile(&buf, 1);
		if (wlen < 0){
			cerr << "Writting data error\n";
			return 0;
		}
	}
	
	return 1;
}


int CServer::Write(const void *buf, size_t nbyte)
{
	return actualChannel->Interface->doWrite(buf, nbyte);
}


int CServer::Read(void *buf, size_t nbyte)
{
	return actualChannel->Interface->doRead(buf, nbyte);
}


int CServer::WriteFile(const void *buf, size_t nbyte)
{
	return write(actualChannel->GetFileDescriptor(), buf, nbyte);
}


int CServer::ReadFile(void *buf, size_t nbyte)
{
	return read(actualChannel->GetFileDescriptor(), buf, nbyte);
}


CServer::~CServer()
{
	//cout << "Close channelData\n";
	for(auto hndlr : channelData){
		hndlr.CloseFile();
		delete hndlr.Interface;
	}
	
	channelData.clear();
	
	cout << "\nDone\n";
}
