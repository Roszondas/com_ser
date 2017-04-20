#include "CServer.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#define	TIMEOUT			3

#define COM_HNDSHAKE	"787\n"
#define COM_READY		"325\n"

enum port_state {WAITING, HNDSHAKED, READING};

CServer::CServer(vector <string> portAdresses)
{
	cout << "CServer\n";
	
	for(auto adr : portAdresses){
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
	
	handler = open("/dev/pts/3", O_RDWR | O_NOCTTY );
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
		for(auto hndlr : portData){
			errFlag = CheckReady(hndlr);
		}
	}
	
	return 0;
	
}





int CServer::CheckReady(PortState handler)
{
	char buf[sizeof(COM_HNDSHAKE)];
	
	cout << "Scan port " << handler.handler << endl;

	int len = read(handler.handler, buf, sizeof(buf));
	
	if(len < 0)
		cerr << "Handshake recieve error.\n";

	
	
	return 0;
}


int CServer::AnswerReady()
{
	return 0;
}


int CServer::RecieveData()
{
	return 0;
}


int CServer::WriteData()
{
	return 0;
}


CServer::~CServer()
{
	cout << "Close portData\n";
	for(auto hndlr : portData){
		cout << "Closing port " << hndlr.handler << "...\n";
		tcsetattr(hndlr.handler, TCSANOW, &savedOptions);
		ioctl(hndlr.handler, TIOCNXCL);
		close(hndlr.handler);
	}
	
	cout << "Done\n";
}
