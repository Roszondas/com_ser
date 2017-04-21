#ifndef COMMONH
#include "Common.h"
#define COMMONH
#endif

#include <sys/types.h>
#include <termios.h>
#include <string>

using namespace std;

class CInterface
{
public:
	virtual int doWrite(const void *buf, size_t nbyte) {return 0;};
	virtual int FindServer() {};
	
	virtual ~CInterface() = 0;
};



class CComInterface : virtual public CInterface
{
private:
	int portHandler = -1;
	struct termios savedOptions;
	
	int OpenPort(string port);
	void ClosePort(int handler);
	int TryHandshake(int handler);
	
public:
	CComInterface() {};
	~CComInterface();
	
	int doWrite(const void *buf, size_t nbyte);
	int FindServer();
};