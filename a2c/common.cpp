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
#include <time.h>
using namespace std; 

struct http_headers {
  const char *path,*method,*protocol;  
  long contentLength;
};      

void unknownexit(int errno) {
    cout<<endl<<"Exiting ..."<<errno<<endl;
    cout.flush();
    exit(1);
};

void error (const char* msg) {
  cout<<"Error "<<msg;
  exit(1);
}

http_headers* parseHttpHeaders(char* buf) {
  http_headers* h = new http_headers();
  FILE *str = fmemopen(buf,strlen(buf),"r");
  // Reading first line to get path and method
  char *saveptr;
  size_t len = 0;
  int j = 0;
  // cout<<"Parsing headers";
  // cout.flush();
  // Can make this a while 
  char *line = NULL ;
  while(getline(&line,&len,str) > 0)
  {
    // cout<<"Read line is "<<line;
    // cout<<"Line is " <<line;
    char *token = NULL ;
    int i = 0;
    bool isContentLength = false;
    do {      
      token = strtok_r(line, " :",&saveptr);      
      line = NULL;
      if (isContentLength) {
        switch(i) {
          //case 0 : isContentLength = false ; break;
        case 1 : 
          char *endptr;
          h->contentLength = strtol(token, &endptr, 10);
          //atoi(token);
          isContentLength = false; break;
        }
      };

      // Depending on i -- infer meaning 
      // if(token != NULL)
      // cout<<"Token "<< token<<endl;
      // cout.flush();
      switch(j) {
      case 0:
        switch (i) {
        case 0 : h->method = token; break;
        case 1 : h->path = token; break;
        case 2 : h->protocol = token; break;
        };break;
      default: {
        switch(i) {
        case 0: if (strcasecmp(token,"Content-Length") == 0)
            isContentLength = true ;                     
        }        
      }
      };
      i++;
    } while (token != NULL);
    j++;
  };
  cout.flush();
  return h;
};

char* readFromSocket(int fd,int &n,void (*cb)(const char*,int fd)) {
  char *str = new char[1];
  char headerBuffer[400];
  char tmp[1],old; 
  int i = 0;
  while (1){
    bzero(tmp,1); 
    recv(fd,tmp,1,0);
    if (tmp[0] == '\n' && old == '\n'){
      break;
    } else {
      headerBuffer[i] = tmp[0];
      old = tmp[0];
    }
    i++;
  };
  http_headers *k = parseHttpHeaders(headerBuffer);
  // cout<<"Header buffer "<<headerBuffer;
  // cout<<endl<<"Content length is "<<k->contentLength;
  long len = 0;
  char buffer[256];
  long flen = k->contentLength;
  do {
    bzero(buffer,256);    
    int rd ;
    if (len+255 < flen){
      rd = 255;
    } else if(len < flen ) {
      rd = flen - len;
    }

    n = recv(fd,buffer,rd, 0);
    len +=n;
    cb(buffer,fd);
    if (flen != 0 && flen <= len){
      break;
    }
  } while (n != 0);
  return str;
};

char* readFromSocketS(int fd,int &n) {
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

char* readFromSocket1(int fd,int &n) {
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
  bool isTime;
  int port,protocol;
};

// HTTP Message 
const char* HTTP_HEADER = "GET %s HTTP/%s \r\n Host: %s \r\n Connection: keep-alive\r\n User-Agent: Mozilla/5.0 (X11; Linux x86_64)\r\n\r\n Accept-Encoding: gzip,deflate,sdch \n\n";


client_args getClientArgs(int argc , char** argv){ 
  char* host , *path ,*tmp2;  
  int n,tmp,c,protocol;
  int port = 1120; // Defauly port
  client_args args ;
  while ((c = getopt (argc, argv, "h:f:p:r:t")) != -1) {
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
    case 't':
      args.isTime = true;
      break;
    case '?':
      if (optopt == 'p')
        cout<<"Enter port info";
      break;
    default:
      cout<<"Option is "<<c <<endl;
    };      
  };
  args.host = host ;
  args.path = path;
  args.port = port;
  args.protocol = protocol;
  return args;
};
// Include files you want to be commonly included - These files can be recurrsively required
#include "fileHandler.cpp"

#endif
