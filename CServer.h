#include <string>
#include <vector>


using namespace std;

class CServer
{
private:
	vector <int> handlers;
	
	int OpenPort(string adr);
	int CheckReady(int handler);
	int AnswerReady();
	int RecieveData();
	int WriteData();
	
public:
	CServer(vector <string> portAdresses);
	~CServer();
	int Start();
};
