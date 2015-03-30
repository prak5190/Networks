#include "common.cpp"
#include <sys/stat.h>
#ifndef __FileHandler__
#define __FileHandler__ 1 
// Information passed to the callback
struct File_stats {
  long seq;
  long chunk_size;
  const char* data;
  const char* fpath;  
  long totalSize;
  long start , offset , buffer_size;
  bool eof;
  ~File_stats() {
    delete data;
    delete fpath;
  }
};

int getFile(const char* fpath,int buffer_size, long start , long offset ,int (*cb) (std::unique_ptr<func_args>), void* forw) {
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
    struct stat fileStat;
    fstat(fileno(fd) , &fileStat);
    long fileSize = fileStat.st_size;
    if (offset < 0)
      offset = -1; // This points to avoid offset

    long chunk_size = offset - start;
    long readSize = 1;
    int seq = 0;    
    while (offset == -1 || chunk_size > 0) {
      if (offset== -1 || chunk_size >= buffer_size) 
        readSize = buffer_size;
      else 
        readSize = chunk_size;
      
      // Populate parameters to be passed to callback
      File_stats *ret = new File_stats();
      ret->fpath = fpath; ret->buffer_size = buffer_size;
      ret->start = start; ret->offset = offset;
      ret->chunk_size = readSize;
      ret->seq = seq;
      ret->totalSize = fileSize;

      std::unique_ptr<func_args> toCb(new func_args());
      toCb->forw = forw;
      if(fread(buffer,readSize, 1, fd) != 0) {
        const char* data =  (string(buffer).c_str());
        ret->data = data ;
        toCb->func = ret;
        cb(std::move(toCb));
        seq++;
        memset(buffer, 0 , sizeof(buffer));
        chunk_size -= readSize;
      } else {
        ret->eof = true;
        toCb->func = ret;
        // Pass eof 
        cb(move(toCb));
        // Reached end of file
        break;
      }
    }     
  };  
  fclose(fd);
  return 0;
}

#endif
