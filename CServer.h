#include <string>
#include <vector>
#include <termios.h>

using namespace std;

struct PortState
{
	int handler;
	int status;
	int file = -1;
	
	PortState(int handler, int status){
		this->handler = handler;
		this->status = status;
	}
};


class CServer
{
private:
	vector <PortState> portData;
	struct termios savedOptions;
	
	int OpenPort(string adr);
	int CheckReady(PortState portState);
	int RecieveData(PortState portState);
	int WriteData(PortState portState, int size);
	int GetFileDescriptor();
	
public:
	CServer(vector <string> portAdresses);
	~CServer();
	int Start();
};
