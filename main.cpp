#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>



#include "CServer.h"
#include "CClient.h"

using namespace std;


void Usage(const char *prog)
{
	printf("Usage: %s [flags]\n"
			"\t-h|? - help\n"
			"\t-s <port> - start as server, listen <port>\n"
			"\t-c <file> - start as client, send content of <file> to server\n", prog);
	
	exit(0);
}


int main(int argc, char **argv)
{
	int options;
	extern char *optarg;
	vector <string> pathStorage;
	
	CClient *Client = nullptr;
	CServer *Server = nullptr;
	
	bool isClient = false;
	bool isServer = false;
	
	while( (options = getopt(argc, argv, "s:c:h?")) != EOF) {
		switch (options) {
			case 's':
				if(isClient)
					continue;
				pathStorage.push_back (optarg);
				isServer = true;
				cout << "Starting as server.\n";
				break;
			case 'c':
				if(isServer)
					continue;
				pathStorage.push_back (optarg);
				isClient = true;
				cout << "Starting as client.\n";
				break;
			case 'h':
			case '?':
			default:
				Usage(argv[0]);	
		}
	}
	
	if(isServer){
		try{
			Server = new CServer(pathStorage);
		}
		catch(int err){
			cerr << "Error " << err << ". Exit.\n";
			if(Server != nullptr)
				delete Server;
			exit(err);
		}
		
		return Server->Start();
	}
	else if(isClient){
		
		try{
			Client = new CClient(pathStorage[0]);
		}
		catch(int err){
			cerr << "Error " << err << ". Exit.\n";
			if(Client != nullptr)
				delete Client;
			exit(err);
		}
		
		return Client->Start();
	}
	else {
		Usage(argv[0]);
	}
	
	if(Client != nullptr)
		delete Client;
	
	if(Server != nullptr)
		delete Server;
	
	return 0;
}
