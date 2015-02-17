#include "common.cpp"
// Returns a chunk of the file 
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
