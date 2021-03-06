CC = g++
CFLAGS = -c -O2 -Wall -g -std=c++14 $(INCLUDES)
SRC_DIR = ./
SOURCES = $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.cpp)  )
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = com_server

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@
	cp $(EXECUTABLE) ../bin/

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *.o *.hex $(EXECUTABLE) ../bin/$(EXECUTABLE)
