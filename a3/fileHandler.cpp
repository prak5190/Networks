#include "common.cpp"
#include <sys/stat.h>
#ifndef __FileHandler__
#define __FileHandler__ 1 

int getFile(const char* fpath , int (*cb)(const char*) /*Means any num of arguments*/ ,             
            int buffer_size, long start , long offset ,int (*cb2) (int)) {
  using namespace std;
  FILE* fd = NULL ;
  fd = fopen(fpath,"r");
  if(NULL == fd)    {
    cout<<"Unable to open file"<<endl;
    return -1;
  } else {
    char buffer[buffer_size];
    size_t len = 0;
    // Start file at start
    fseek(fd,start,SEEK_SET);
    memset(buffer, 0 , sizeof(buffer));

    if (offset < 0)
      offset = -1; // This points to avoid offset

    long totalSize = offset - start;
    long readSize = 1;
    int seq = 0;
    while (offset == -1 || totalSize > 0) {
      if (offset== -1 || totalSize >= buffer_size) 
        readSize = buffer_size;
      else 
        readSize = totalSize;
      if(fread(buffer,readSize, 1, fd) != 0) {            
        // TODO string size sent to CB in case of lesser bytes needs to reduced to avoid junk
        cb2(seq);
        cb(buffer);
        seq++;
        memset(buffer, 0 , sizeof(buffer));
        totalSize -= readSize;
      } else {
        // Reached end of file
        break;
      }
    }     
  };  
  return 0;
}

#endif
