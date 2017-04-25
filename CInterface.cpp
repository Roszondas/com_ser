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

vector <string> portListInterface {"/dev/pts/5", "/dev/pts/3", "/dev/pts/2", "/dev/pts/4"};

CInterface::~CInterface() {};


int CComInterface::OpenPort(string port)
{
	ClosePort();
	
	cout << "\nPort " << port << " opening ";
	
	portHandler = open(port.c_str(), O_RDWR | O_NOCTTY );

	if (portHandler == -1) {
		cerr << "failed.\nError opening port " << port << endl;
		return 0;
	}
	
	if (ioctl(portHandler, TIOCEXCL)) {
		cerr << "failed.\nPort is busy\n";
		close (portHandler);
		return 0;
	}
	
	struct termios options;
	tcgetattr(portHandler, &options);
	savedOptions = options;
	
	cfsetispeed(&options, B19200);
	cfsetospeed(&options, B19200);
	
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag &= ~(PARENB | PARODD);
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_iflag &= ~IGNBRK;
	options.c_lflag &= ~(ICANON);
	options.c_cc[VMIN]  = 0;
	options.c_cc[VTIME] = 5;
	
	if( tcsetattr(portHandler, TCSANOW, &options) != 0){
		cerr << "failed.\nError writing port options\n";
		ioctl(portHandler, TIOCNXCL);
		close (portHandler);
		return 0;
	}
	
	// int flags = fcntl(portHandler, F_GETFL, 0);
	// fcntl(portHandler, F_SETFL, flags | O_NONBLOCK);
	
	cout << "successful.\n";
	
	portName = port;
	
	return 1;
}


void CComInterface::ClosePort(){
	if(portHandler < 0) return;
	cout << "Closing port\n";
	errno = 0;
	tcsetattr(portHandler, TCSANOW, &savedOptions);
	ioctl(portHandler, TIOCNXCL);
	close (portHandler);
}


int CComInterface::doWrite(const void *buf, size_t nbyte) 
{
	//cout << "Write to " << portHandler << " Len " << nbyte << endl;
	int len = write(portHandler, buf, nbyte);
	if (len < 0) {
		cerr << "Write error.\n";
		//return 0;
	}
	//cout << "Actual len " << len << endl;
	return len;
}


int CComInterface::doRead(void *buf, size_t nbyte) 
{
	//cout << "Read from " << portHandler << " Len " << nbyte << endl;
	int len = read(portHandler, buf, nbyte);

	if (len < 0) {
		cerr << "Read error.\n";
		//return 0;
	}
	
	//cout << "Actual len " << len << endl;
	return len;
}


CComInterface::~CComInterface()
{
	if(portHandler != -1){
		cout << "Closing port: " << portName << endl;
		tcsetattr(portHandler, TCSANOW, &savedOptions);
		ioctl(portHandler, TIOCNXCL);
		close (portHandler);
		portName = "";
	}
}
