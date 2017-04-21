#ifndef COMMONH
#include "Common.h"
#define COMMONH
#endif

#include <string>
#include <termios.h>

#ifndef CINTERFASEH 
#include "CInterface.h"
#define CINTERFASEH 
#endif

using namespace std;

class CClient
{
private:
	int portHandler = -1;
	int fileHandler = -1;
	string filePath;
	struct termios savedOptions;
	CInterface *Interface = nullptr;
	
	int FindServer();
	int TryHandshake();
	bool isFileExist(string filePath);
	int Transmit();
	int SendReady();
	int WaitReady();
	int SendData();
	
	int Write(const void *buf, size_t nbyte);
	
public:
	CClient(string filePath, int protocol);
	~CClient();
	int Start();
};
