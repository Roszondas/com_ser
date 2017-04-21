#ifndef CINTERFASEH
#include "CInterface.h"
#define CINTERFASEH
#endif

#include <iostream>

#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <vector>

vector <string> portListInterface {"/dev/pts/5", "/dev/pts/2", "/dev/pts/3", "/dev/pts/4"};

CInterface::~CInterface() {};


int CComInterface::FindServer()
{
	int handler = -1;

	for(unsigned int index = 0; index < portListInterface.size(); index++){
		handler = OpenPort(portListInterface[index]);
		if(handler != -1) {
			if(TryHandshake(handler) == -1)
				ClosePort(handler);
			else
				break;
		}
	}

	portHandler = handler;
	
	return handler;
}


int CComInterface::TryHandshake(int handler)
{
	cout << "Trying to find server\n";
	
	int len = write(handler, COM_HNDSHAKE, sizeof(COM_HNDSHAKE));
	if (len != sizeof(COM_HNDSHAKE)) {
		cerr << "Handshake send error.\n";
	}
	
	char buf[sizeof(COM_HNDSHAKE)];
	
	for(int i = 0; i < TIMEOUT; i++){
		cout << "Scaning\n";
		len = read(handler, buf, sizeof(buf));
		
		if(len > 0)
			break;
		
		if(len < 0){
			cerr << "Handshake recieve error.\n";
			return -1;
		}
	}
	
	cerr << "Len " << len << " Get " << buf << " wait " << COM_HNDSHAKE << endl;
	
	if(!strcmp(buf, COM_HNDSHAKE)){
		cout << "Server found\n";
		errno = 0;
		return 0;
	}
	
	cout << "Server not found at " << handler << ". Check next port\n";
	
	return -1;
}


int CComInterface::OpenPort(string port)
{
	cout << "OpenPort() in\n";
	int handler = -1;
	
	handler = open(port.c_str(), O_RDWR | O_NOCTTY );
	
	cout << "OpenPort() open\n";
	
	if (handler == -1) {
		cerr << "Error opening port " << port << endl;
		return -1;
	}
	
	if (ioctl(handler, TIOCEXCL)) {
		cerr << "Port is busy\n";
		return -1;
	}
	
	cout << "OpenPort() termios\n";
	
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
	options.c_iflag &= ~O_NONBLOCK ;
	options.c_cc[VMIN]  = 0;
	options.c_cc[VTIME] = 5;
	
	
	
	if( tcsetattr(handler, TCSANOW, &options) != 0){
		cerr << "Error writing port options\n";
		return -1;
	}
	
	return handler;
}


void CComInterface::ClosePort(int handler){
	if(handler < 0) return;
	
	tcsetattr(handler, TCSANOW, &savedOptions);
	ioctl(handler, TIOCNXCL);
	close (handler);
}


int CComInterface::doWrite(const void *buf, size_t nbyte) 
{
	return 0;
}


CComInterface::~CComInterface()
{
	cout << "Closing port " << portHandler << "...\n";
	ClosePort(portHandler);
}
