CC=g++ -std=c++11
CPFLAGS=-g -Wall
LDFLAGS= -lcrypto -lpthread

CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(addprefix ,$(notdir $(CPP_FILES:.cpp=.o)))
SRC= bt_client.cpp bt_lib.c bt_setup.c
OBJ=$(SRC:.c=.o)
BIN=bt_client

all: $(BIN)

$(BIN): $(OBJ_FILES) $(OBJ)	
	$(CC) $(CPFLAGS) -o $(BIN) $(OBJ)  $(LDFLAGS)

%.o: %.cpp
	$(CC) -c $(CPFLAGS) -c -o $@ $<	
%.o:%.c
	$(CC) -c $(CPFLAGS) -o $@ $<

$(SRC):

clean:
	rm -rf $(BIN) $(OBJ_FILES)
