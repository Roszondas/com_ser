#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>

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
		
	}
	else if(isClient){
		
	}
	else {
		Usage(argv[0]);
	}
		
	return 0;
}
