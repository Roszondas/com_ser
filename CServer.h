#ifndef COMMONH
#include "Common.h"
#define COMMONH
#endif

#ifndef CINTERFASEH 
#include "CInterface.h"
#define CINTERFASEH 
#endif

#include <string>
#include <vector>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

struct ChannelState
{
	int handler;
	string portName;
	int status;
	int file = -1;
	CInterface* Interface;
	
	ChannelState(CInterface* Interface, int status){
		this->Interface = Interface;
		this->status = status;
		handler = Interface->ReturnPort();	//temporary
		portName = Interface->GetPortName();
	}
	
	int GetFileDescriptor(){
		if (file != -1)
			return file;
		
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);

		char name[255];
		sprintf(name, "%d-%d-%d_%d-%d-%d.hex", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		
		file = open(name, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);

		return file;
	}
	
	void CloseFile(){
		if (file != -1)
			close(file);
		
		file = -1;
	}
	
};


class CServer
{
private:
	vector <ChannelState> channelData;
	ChannelState* actualChannel;
	struct termios savedOptions;
	
	int CheckReady();
	int RecieveData();
	int WriteData(int size);
	
	int Write(const void *buf, size_t nbyte);
	int Read(void *buf, size_t nbyte);
	
	int WriteFile(const void *buf, size_t nbyte);
	int ReadFile(void *buf, size_t nbyte);
	
public:
	CServer(vector <string> portAdresses, int protocol);
	~CServer();
	int Start();
};
