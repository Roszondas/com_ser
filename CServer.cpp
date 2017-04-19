#include "CServer.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

CServer::CServer(vector <string> portAdresses)
{
	for(auto adr : portAdresses){
		int handler = OpenPort(adr);
		if(handler == -1){
			cerr << "Can't open port " << adr << endl;
			continue;
		}
		handlers.push_back(handler);
	}
	
	if(handlers.empty())
		throw ENXIO;
}


int CServer::OpenPort(string adr)
{
	return 0;
}


int CServer::Start()
{
	int errFlag = 0;
	
	while(!errFlag){
		for(auto hndlr : handlers){
			CheckReady(hndlr);
		}
	}
	
	return 0;
	
}





int CServer::CheckReady(int handler)
{
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
	for(auto hndlr : handlers){
		close(hndlr);
	}
}
