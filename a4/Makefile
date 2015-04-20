CC=g++ -std=c++11
CPFLAGS=-g -Wall
LDFLAGS= -lcrypto -lpthread

# CPP_FILES := $(wildcard *.cpp)	
# OBJ_FILES := $(addprefix ,$(notdir $(CPP_FILES:.cpp=.o)))
SRC= bt_client.cpp
OBJ=$(SRC:.cpp=.o)
BIN=bt_client

all: 	
	$(CC) $(CPFLAGS) -o $(BIN) $(SRC) $(LDFLAGS)

clean:
	rm -rf $(BIN) 
