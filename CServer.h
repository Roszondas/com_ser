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

using namespace std;

struct ChannelState
{
	int handler;
	int status;
	int file = -1;
	CInterface* Interface;
	
	ChannelState(int handler, int status){
		this->handler = handler;
		this->status = status;
	}
	
	ChannelState(CInterface* Interface, int status){
		this->Interface = Interface;
		this->status = status;
		handler = Interface->ReturnPort();	//temporary
	}
};


class CServer
{
private:
	vector <ChannelState> channelData;
	ChannelState* actualChannel;
	struct termios savedOptions;
	
	int CheckReady(ChannelState &portState);
	int RecieveData();
	int WriteData(ChannelState portState, int size);
	int GetFileDescriptor();
	
public:
	CServer(vector <string> portAdresses, int protocol);
	~CServer();
	int Start();
};
