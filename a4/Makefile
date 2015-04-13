CC=g++ -std=c++11
CPFLAGS=-g -Wall
LDFLAGS= -lcrypto


SRC= bt_client.cpp bt_lib.c bt_setup.c 
OBJ=$(SRC:.c=.o)
BIN=bt_client

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CPFLAGS) -o $(BIN) $(OBJ)  $(LDFLAGS)


%.o:%.c
	$(CC) -c $(CPFLAGS) -o $@ $<  

$(SRC):

clean:
	rm -rf $(BIN)
