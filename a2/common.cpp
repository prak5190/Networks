#ifndef __COMMON__
#define __COMMON__ 1 

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdarg.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h> 

using namespace std; 

void error (const char* msg) {
  cout<<"Error "<<msg;
  exit(1);
}

char* readFromSocket(int fd,int &n) {
  char *str = new char[1];
  char buffer[256];
  do {
    bzero(buffer,256);    
    n = read(fd,buffer,255);
    char *newstr = new char [strlen(str) + strlen(buffer)];    
    strcat(newstr ,str);
    str = strcat(newstr ,buffer);
    cout.flush();    
  } while (n > 0 && n == 255);
  return str;
}

int writeToClient(const char* buf , int fd) {
  write(fd,buf,strlen(buf));
  return 0;
};

// Include files you want to be commonly included - These files can be recurrsively required
#include "fileHandler.cpp"

#endif
