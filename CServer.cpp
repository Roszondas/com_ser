#include "CServer.h"
#include <iostream>
#include <errno.h>
#include <sys/ioctl.h>



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
			switch(actualChannel->status){
				case READING:	
					errFlag = RecieveData();
					break;
					
				case WAITING:
					errFlag = WaitHandshake();
					break;
					
				case HNDSHAKED:	
				default:
					errFlag = CheckReady();
			}
		}
	}
	
	return errFlag;
}


int CServer::WaitHandshake()
{
	int size = sizeof(COM_HNDSHAKE);
	char buf[size];
	memset(buf, '\0', size);
	
	//cout << "Scan port " << actualChannel->portName << endl;

	int len = Read(buf, size);
	if(len < 0){
		cerr << "Handshake recieve error.\n";
	}

	//cerr << "Len " << len << " Get " << buf << " wait " << COM_HNDSHAKE "\n " << strcmp(buf, COM_HNDSHAKE) << endl;
	
	if(!strcmp(buf, COM_HNDSHAKE)){
		len = 0;
		
		len = Write(COM_HNDSHAKE, sizeof(buf));
		if (len < 0) {
			cerr << "Writing confirm ready failed\n";
			return 0;
		}
		
		cout << "Port " << actualChannel->portName << " recieved handshake. Waiting for confirmation.\n";
		
		actualChannel->status = HNDSHAKED;
	}
	
	return 1;
}


int CServer::CheckReady()
{
	int size = sizeof(COM_READY);
	char buf[size];
	memset(buf, '\0', size);
	
	//cout << "Wait confirmation from " << actualChannel->portName << endl;

	int len = 0;
	memset(buf, 0, sizeof(buf));
	

	len = Read(buf, sizeof(buf));
	
	if (len < 0){
		cerr << "Confirmation recieve error\n";
		return 0;
	}

	
	//cerr << "Len " << len << " Get " << buf << " wait " << COM_READY << endl;
	
	if(!strcmp(buf, COM_READY)){
		len = Write(COM_READY, sizeof(COM_READY));
		if (len < 0) {
			cerr << "Writing ready signal failed\n";
			return 0;
		}
		
		actualChannel->status = READING;
		cout << "Confirmation recieved. Ready to write data. Status " << actualChannel->status << endl;
	}
	
	return 1;
}


int CServer::RecieveData()
{
	int err = 0;
	
	//cout << "Waiting data from " << actualChannel->portName << endl;

	char *dataBuf = actualChannel->buffer;
	char signalBuf[sizeof(COM_END)];
	memset(signalBuf, '\0', sizeof(dataBuf));

	int tm = 0;
	
	while (tm < END_TIMEOUT){
		err = Read(&dataBuf[sizeof(dataBuf) - 1], 1);
		
		if (err < 0){
			cerr << "Recieving data error\n";
			return 0;
		}
		else if(err == 0){
			tm++;
			continue;
			//return 1;
		}
		else {
			actualChannel->dataLen += err;
			for(int i = 0; i < sizeof(dataBuf) - 1; i++){
				dataBuf[i] = dataBuf[i+1];
				signalBuf[i] = dataBuf[i];
			}
		}
			
		
		if(!strcmp(signalBuf, COM_END)){
			actualChannel->CloseFile();
			actualChannel->status = WAITING;
			actualChannel->dataLen = 0;
			cout << "Data recieved and saved\n\n";
			return 1;
		}
		else if(actualChannel->dataLen > sizeof(COM_END) - 2) {
			err = WriteFile(&dataBuf[0], 1);
			//cout << dataBuf[0] << " " << err << "\n";
			if (err < 0){
				cerr << "Writting data error\n";
				return 0;
			}
		}
	}
	
	actualChannel->CloseFile();
	actualChannel->status = WAITING;
	actualChannel->dataLen = 0;
	cout << "Transmission timed out\n\n";
	
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
