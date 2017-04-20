#include <string>
#include <vector>
#include <termios.h>

using namespace std;

struct PortState
{
	int handler;
	int status;
	
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
	int CheckReady(PortState handler);
	int AnswerReady();
	int RecieveData();
	int WriteData();
	
public:
	CServer(vector <string> portAdresses);
	~CServer();
	int Start();
};
