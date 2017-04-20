#include "CServer.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>

#define	TIMEOUT			3

#define COM_HNDSHAKE	"787\n"
#define COM_READY		"325\n"

enum port_state {WAITING, HNDSHAKED, READING};

CServer::CServer(vector <string> portAdresses)
{
	cout << "CServer\n";
	
	for(auto &adr : portAdresses){
		int handler = OpenPort(adr);
		if(handler == -1){
			cerr << "Can't open port " << adr << endl;
			continue;
		}
		portData.push_back(PortState(handler, WAITING));
	}
	
	if(portData.empty()){
		cerr << "There is no working port\n";
		throw ENXIO;
	}
}


int CServer::OpenPort(string adr)
{
	int handler = -1;
	
	cout << "Establishing server\n";
	
	handler = open(adr.c_str(), O_RDWR | O_NOCTTY );
	if (handler == -1) {
		cerr << "Error opening port\n";
		return errno;
	}
	
	if (ioctl(handler, TIOCEXCL)) {
		cerr << "Port is busy\n";
		return errno;
    }
	
	struct termios options;
	tcgetattr(handler, &options);
	savedOptions = options;
	
	cfsetispeed(&options, B19200);
	cfsetospeed(&options, B19200);
	
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag &= ~(PARENB | PARODD);
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_iflag &= ~IGNBRK;
	options.c_cc[VMIN]  = 0;
	options.c_cc[VTIME] = 5;
	
	if( tcsetattr(handler, TCSANOW, &options) != 0){
		cerr << "Error writing port options\n";
		return -1;
	}
		
	return handler;
}


int CServer::Start()
{
	int errFlag = 0;

	while(!errFlag){
		for(auto &hndlr : portData){
			if(hndlr.status != READING){
				errFlag = CheckReady(hndlr);
			}
			else{
				errFlag = RecieveData(hndlr);
			}
		}
	}
	
	return 0;
	
}


int CServer::CheckReady(PortState &portState)
{
	char buf[sizeof(COM_HNDSHAKE)];
	
	cout << "Scan port " << portState.handler << endl;

	int len = read(portState.handler, buf, sizeof(buf));
	
	if(len < 0)
		cerr << "Handshake recieve error.\n";

	if(len > 0 && !strcmp(buf, COM_HNDSHAKE)){
		len = 0;
		cout << "Port " << portState.handler << " recieved handshake. Waiting for confirmation.\n";
		
		len = write(portState.handler, COM_HNDSHAKE, sizeof(COM_HNDSHAKE));
		if (len != sizeof(COM_HNDSHAKE)) {
			cerr << "Writing confirm ready failed\n";
		}
	
		len = 0;
	
		memset(buf, 0, sizeof(buf));
		for(int i = 0; i < TIMEOUT; i++){
			len = read(portState.handler, buf, sizeof(buf));
			
			if (len > 0)
				break;
			else if (len < 0)
				cerr << "Confirmation recieve error\n";
		}
		cerr << "Len " << len << " Get " << buf << " wait " << COM_READY << endl;
		if(len > 0 && !strcmp(buf, COM_READY)){
			len = write(portState.handler, COM_READY, sizeof(COM_READY));
			if (len != sizeof(COM_READY)) {
				cerr << "Writing ready signal failed\n";
			}
			portState.status = READING;
			cout << "Confirmation recieved. Ready to write data. Status " << portState.status << endl;
		}
	}
	
	return 0;
}


int CServer::RecieveData(PortState &portState)
{
	char buf[sizeof(int)];
	memset(buf, 0, sizeof(buf));
	int err = 0;
	
	cout << "Waiting data " << portState.handler << endl;
	
	int len = 0;
	
	while(len < sizeof(buf)){
		len += read(portState.handler, buf, sizeof(buf));
		if (len < 0){
			cerr << "Recieving size error\n";
			err = errno;
			return err;
		}
	}
	
	cout << "buf = " << buf << endl;
	int msgSize = atoi(buf);
	cout << "Ready to recieve " << msgSize << " bytes\n";
	
	portState.file = GetFileDescriptor();
	
	if(portState.file == -1){
		cout << "File creation failed\n";
		err = errno;
	}
	else{
		err = WriteData(portState, msgSize);
		close(portState.file);
		portState.status = WAITING;
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


int CServer::WriteData(PortState portState, int size)
{
	char *buf = new char[size];
	int err = 0;
	int len = 0;
	
	while(len < size){
		len += read(portState.handler, buf, size);
		if (len < 0){
			cerr << "Recieving data error\n";
			err = errno;
			return err;
		}
	
		cout << "Read " << len << " bytes\n";
		cout << "Data: " << buf << endl;
	
		int wlen = write(portState.file, buf, len);
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
	cout << "Close portData\n";
	for(auto hndlr : portData){
		cout << "Closing port " << hndlr.handler << "...\n";
		tcsetattr(hndlr.handler, TCSANOW, &savedOptions);
		ioctl(hndlr.handler, TIOCNXCL);
		close(hndlr.handler);
		close(hndlr.file);
	}
	
	cout << "Done\n";
}
