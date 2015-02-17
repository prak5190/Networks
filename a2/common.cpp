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
#include <cstdarg>
#include <error.h>
using namespace std; 

void unknownexit(int errno) {
    cout<<endl<<"Exiting ..."<<errno<<endl;
    cout.flush();
    exit(1);
};

void error (const char* msg) {
  cout<<"Error "<<msg;
  exit(1);
}

char* readFromSocket(int fd,int &n,void (*cb)(const char*,int fd)) {
  char *str = new char[1];
  char buffer[256];
  do {
    bzero(buffer,256);    
    n = recv(fd,buffer,255, 0);
    cb(buffer,fd);
  } while (n != 0);
  return str;
}
char* readFromSocket(int fd,int &n) {
  char *str = new char[1];
  char buffer[256];
  do {
    bzero(buffer,256);    
    n = recv(fd,buffer,255, 0);
    char *newstr = new char [strlen(str) + strlen(buffer)];    
    strcat(newstr ,str);
    str = strcat(newstr ,buffer);
    cout.flush();    
  } while (n != 0);
  return str;
}

int writeToClient(const char* buf , int fd) {
  send(fd,buf,strlen(buf),0);
  return 0;
};

// Use this to debug 
int logLevel = 3;
void log(int level,int nargs , ...) {
  va_list ap;
  va_start (ap , nargs);
  if (level <= logLevel){
    for (int i = 1; i <= nargs ; i++){
      const char* a = va_arg(ap,const char*);
      cout<<a;
    }
  }
  va_end(ap);
};

struct client_args {
  char *host , *path;
  int port,protocol;
};

// HTTP Message 
const char* HTTP_HEADER = "GET %s HTTP/%s \r\n Host: %s \r\n Connection: keep-alive\r\n User-Agent: Mozilla/5.0 (X11; Linux x86_64)\r\n\r\n Accept-Encoding: gzip,deflate,sdch \n\n";


client_args getClientArgs(int argc , char** argv){ 
  char* host , *path ,*tmp2;  
  int n,tmp,c,protocol;
  int port = 1120; // Defauly port
  while ((c = getopt (argc, argv, "h:f:p:r:")) != -1) {
    switch(c) {
    case 'p' : 
      tmp = atoi(optarg);
      if(tmp != 0) {
        port = tmp;
        cout<<"Binding port to "<<port << endl;
      }
      break;
    case 'r' : 
      tmp = atoi(optarg);
      protocol = tmp;
      if(tmp != 0) {
        protocol = 1;
      } else {
        protocol = 0;
      }
      cout<<"Protocol HTTP 1."<<protocol << endl;       
      break;
    case 'h' : 
      tmp2 = optarg;
      n = sizeof(optarg);
      host = new char [n];
      strcpy(host , optarg);
      cout<<"Host is :  "<< host << endl;
      break;
    case 'f' : 
      tmp2 = optarg;
      n = sizeof(optarg);
      path = new char [n];
      strcpy(path , optarg);
      cout<<"File is :  "<< path << endl;
      break;
    case '?':
      if (optopt == 'p')
        cout<<"Enter port info";
      break;
    default:
      cout<<"Option is "<<c <<endl;
    };      
  };
  client_args args ;
  args.host = host ;
  args.path = path;
  args.port = port;
  args.protocol = protocol;
  return args;
};
// Include files you want to be commonly included - These files can be recurrsively required
#include "fileHandler.cpp"

#endif
