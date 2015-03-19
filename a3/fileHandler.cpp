#include "common.cpp"
#include <sys/stat.h>
#ifndef __FileHandler__
#define __FileHandler__ 1 

int getFile(const char* fpath , int (*cb)(const char*) /*Means any num of arguments*/ , 
            int size) {
  using namespace std;
  FILE* fd = NULL ;
  fd = fopen(fpath,"r");
  if(NULL == fd)    {
    cout<<"Unable to open file"<<endl;
    return -1;
  } else {
    char buffer[size];
    size_t len = 0;
    memset(buffer, 0 , sizeof(buffer));
    while(fread(buffer,sizeof(buffer) , 1, fd) != 0) {      
      // cout<<buffer;
      // Need to pass to cb;
      cb(buffer);
      memset(buffer, 0 , sizeof(buffer));
    };
  };  
  return 0;
}

#endif
