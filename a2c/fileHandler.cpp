#include "common.cpp"
#include <sys/stat.h>
#ifndef __FileHandler__
#define __FileHandler__ 1 

int getFile(const char* fpath , int (*cb)(const char*,int fd) , int sfd) {
  FILE* fd = NULL ;   
  fd = fopen(fpath,"r");
  if(NULL == fd)    {
    cout<<"Unable to open file"<<endl;
    return -1;
  } else {
    char * line = NULL;
    size_t len = 0;
    struct stat st;
    stat(fpath, &st);
    int size = st.st_size;
    char s[100];
    sprintf(s,"HTTP/1.1 200 OK\nContent-Length:%d \nServer: TestServer \nContent-Type:text\n\n",size);
    cb(s,sfd);
    while(getline(&line, &len, fd) != -1) {
      //cout<<line<<endl;      
      cb(line,sfd);
    };
  };
  
  return 0;
}

int getFile(const char* fpath , int (*cb)(const char* buf , int fd , sockaddr* clientaddr,socklen_t clientlen) , int sfd , sockaddr* clientaddr,socklen_t clientlen) {
  FILE* fd = NULL ;   
  fd = fopen(fpath,"r");
  if(NULL == fd)    {
    cout<<"Unable to open file"<<endl;
    return -1;
  } else {
    char * line = NULL;
    size_t len = 0;
    while(getline(&line, &len, fd) != -1) {
      //cout<<line<<endl;      
      cb(line,sfd,clientaddr,clientlen);
    };
  };
  
  return 0;
}

#endif
