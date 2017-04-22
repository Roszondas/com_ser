#include "CServer.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
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
}


int CServer::Start()
{
	int errFlag = 0;

	while(!errFlag){
		for(auto &hndlr : channelData){
			actualChannel = &hndlr;
			switch(hndlr.status){
				case READING:	errFlag = RecieveData();
				case WAITING:
				case HNDSHAKED:	
				default:		errFlag = CheckReady(hndlr);
			}
		}
	}
	
	return errFlag;
}


int CServer::CheckReady(ChannelState &channelState)
{
	char buf[sizeof(COM_HNDSHAKE)];
	
	cout << "Scan port " << channelState.handler << endl;

	int len = read(channelState.handler, buf, sizeof(buf));
	
	if(len < 0)
		cerr << "Handshake recieve error.\n";

	if(len > 0 && !strcmp(buf, COM_HNDSHAKE)){
		len = 0;
		cout << "Port " << channelState.handler << " recieved handshake. Waiting for confirmation.\n";
		
		len = write(channelState.handler, COM_HNDSHAKE, sizeof(COM_HNDSHAKE));
		if (len != sizeof(COM_HNDSHAKE)) {
			cerr << "Writing confirm ready failed\n";
		}
	
		len = 0;
	
		memset(buf, 0, sizeof(buf));
		for(int i = 0; i < TIMEOUT; i++){
			len = read(channelState.handler, buf, sizeof(buf));
			
			if (len > 0)
				break;
			else if (len < 0)
				cerr << "Confirmation recieve error\n";
		}
		cerr << "Len " << len << " Get " << buf << " wait " << COM_READY << endl;
		if(len > 0 && !strcmp(buf, COM_READY)){
			len = write(channelState.handler, COM_READY, sizeof(COM_READY));
			if (len != sizeof(COM_READY)) {
				cerr << "Writing ready signal failed\n";
			}
			channelState.status = READING;
			cout << "Confirmation recieved. Ready to write data. Status " << channelState.status << endl;
		}
	}
	
	return 0;
}


int CServer::RecieveData()
{
	ChannelState *channelState = actualChannel;
	
	char buf[sizeof(int) + 1];
	memset(buf, 0, sizeof(buf));
	int err = 0;
	
	cout << "Waiting data " << channelState->handler << endl;
	
	int len = 0;
	
	while(len < sizeof(buf)){
		len += read(channelState->handler, buf, sizeof(buf));
		if (len < 0){
			cerr << "Recieving size error\n";
			err = errno;
			return err;
		}
	}
	
	cout << "buf = " << buf << endl;
	int msgSize = atoi(buf);
	cout << "Ready to recieve " << msgSize << " bytes\n";
	
	channelState->file = GetFileDescriptor();
	
	if(channelState->file == -1){
		cout << "File creation failed\n";
		err = errno;
	}
	else{
		err = WriteData(*channelState, msgSize);
		close(channelState->file);
		channelState->status = WAITING;
	}
	
	return err;
}


int CServer::GetFileDescriptor()
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	char name[255];
	sprintf(name, "%d-%d-%d_%d-%d-%d.hex", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	
	int fd = open(name, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);

	return fd;
}


int CServer::WriteData(ChannelState channelState, int size)
{
	//char *buf = new char[size];
	char buf;
	int err = 0;
	int len = 0;
	
	cout << "Reading data.\n";
	while(len < size){
		len += read(channelState.handler, &buf, 1);
		if (len < 0){
			cerr << "Recieving data error\n";
			err = errno;
			return err;
		}
	
		//cout << "Read " << len << " bytes\n";
		//cout << "Data: " << buf << endl;
	
		int wlen = write(channelState.file, &buf, 1);
		if (wlen < 0){
			cerr << "Writting data error\n";
			err = errno;
			return err;
		}
	}
	
	return err;
}


CServer::~CServer()
{
	cout << "Close channelData\n";
	for(auto hndlr : channelData){
		delete hndlr.Interface;
		cout << "Closing port " << hndlr.handler << "...\n";
		tcsetattr(hndlr.handler, TCSANOW, &savedOptions);
		ioctl(hndlr.handler, TIOCNXCL);
		close(hndlr.handler);
		close(hndlr.file);
	}
	
	channelData.clear();
	
	cout << "Done\n";
}
