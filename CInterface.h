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
	virtual int ReturnPort() {return 0;};
	virtual string GetPortName() {return (string)" ";};
	
	virtual int doWrite(const void *buf, size_t nbyte) {return 0;};
	virtual int doRead(void *buf, size_t nbyte) {return 0;};
	virtual int OpenPort(string port){return 0;};
	virtual void ClosePort() {};
	
	virtual ~CInterface() = 0;
};



class CComInterface : virtual public CInterface
{
private:
	int portHandler = -1;
	struct termios savedOptions;
	string portName = "";

public:
	string GetPortName() {return portName;};
	
	CComInterface() {};
	~CComInterface();
	
	int doWrite(const void *buf, size_t nbyte);
	int doRead(void *buf, size_t nbyte);
	int OpenPort(string port);
	void ClosePort();
};