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

#define	TIMEOUT			3

#define COM_HNDSHAKE	"787\n"
#define COM_READY		"325\n"

using namespace std;

CClient::CClient(string filePath)
{
	if(!isFileExist(filePath))
		throw ENOENT;
	this->filePath = filePath;
}


bool CClient::isFileExist(string filePath)
{
	return true;
}


int CClient::Start()
{
	portHandler = FindFreePort();
	
	cout << "Start " << portHandler << endl;
	
	if(portHandler == -1) return ENXIO;
	
	int res = Transmit();
	
	cout << "Start exiting " << portHandler << endl;
	
	return res;
}


int CClient::FindFreePort()
{
	int handler = -1;
	
	handler = open("/dev/pts/2", O_RDWR | O_NOCTTY );
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
	options.c_cc[VMIN]  = 0;
	options.c_cc[VTIME] = 5;
	
	cout << "Trying to find server\n";
	
	if( tcsetattr(handler, TCSANOW, &options) != 0){
		cerr << "Error writing port options\n";
		return -1;
	}
		
	int len = write(handler, COM_HNDSHAKE, sizeof(COM_HNDSHAKE));
    if (len != sizeof(COM_HNDSHAKE)) {
        cerr << "Handshake send error.\n";
    }
    tcdrain(handler);
	
	char buf[sizeof(COM_HNDSHAKE)];
	
	for(int i = 0; i < TIMEOUT; i++){
		cout << "Scaning\n";
		len = read(handler, buf, sizeof(buf));
		if(len > 0)
			break;
		else if(len < 0)
			cerr << "Handshake recieve error.\n";
	}
	
	if(!strcmp(buf, COM_HNDSHAKE))
		cout << "Server found\n";
	else
		cout << "There will be searching for other port\n";
	
	return handler;
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
		printf("Writing error: %d: %s\n", len, strerror(errno));
    }
    tcdrain(portHandler);
	
	return 0;
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
	cout << "Done\n";
}