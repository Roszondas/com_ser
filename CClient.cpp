#include "CClient.h"

#include <iostream>
#include <string.h>
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

using namespace std;

CClient::CClient(string filePath)
{
	if(!isFileExist(filePath))
		throw ENOENT;
	this->filePath = filePath;
}


CClient::~CClient()
{
	if(portHandler != -1)
		close (portHandler);
}

int CClient::Start()
{
	int portHandler = FindFreePort();
	
	if(portHandler == -1) return ENXIO;
	
	return Transmit();
}

bool CClient::isFileExist(string filePath)
{
	return true;
}


int CClient::FindFreePort()
{
	int handler = 0;
	return handler;
}


int CClient::Transmit()
{
	SendReady();
	if(WaitReady() == -1)
		return ENOMSG;
	
	return SendData();
}


int CClient::SendReady()
{
	return 0;
}


int CClient::WaitReady()
{
	return 0;
}


int CClient::SendData()
{
	return 0;
}
