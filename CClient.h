#include <string>
#include <termios.h>


using namespace std;

class CClient
{
private:
	int portHandler = -1;
	int fileHandler = -1;
	string filePath;
	struct termios savedOptions;
	
	bool isFileExist(string filePath);
	int FindFreePort();
	int Transmit();
	int SendReady();
	int WaitReady();
	int SendData();
	
public:
	CClient(string filePath);
	~CClient();
	int Start();
};
