CC = g++
CFLAGS = -c -O2 -Wall -g $(INCLUDES)
SRC_DIR = ./
SOURCES = $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.cpp)  )
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = com_server

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
