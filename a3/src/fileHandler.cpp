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
  bool isFstat;
  ~File_stats() {
    delete data;
    delete fpath;
  }
};

std::unordered_map<string,FILE*> FileDescriptorMap;
int closeFile(string name) {
 auto search = FileDescriptorMap.find(name);
  FILE* fd = NULL;
  bool isMapped = false;
  if (search != FileDescriptorMap.end()) {
    fd = search->second;
    fclose(fd);
    // Delete from map
    FileDescriptorMap.erase(name);
  }
}
// Will pad data to the chunksize 
int assembleFile (string name,int start, char* data , long size , bool isLast) {
  using namespace std;
  auto search = FileDescriptorMap.find(name);
  FILE* fd = NULL;
  bool isMapped = false;
  if (search != FileDescriptorMap.end()) {
    fd = search->second;
    isMapped = true;
  } else {
    // Open the file -- Create if not present
    fd = std::fopen(name.c_str(), "wb");
    FileDescriptorMap.insert(std::make_pair(name , fd));
  }
  if (start < 0 || ftell(fd) != start) {
    fseek(fd , start,SEEK_SET);
  };
  if (fwrite(data,sizeof(char),size, fd) != 0){
    // File writing done
    //std::cout << "File writing done " << std::endl;
  }
  if (isLast) {
    fclose(fd);
    // Delete from map
    FileDescriptorMap.erase(name);
  } else if(!isMapped){
    FileDescriptorMap.insert(std::make_pair(name , fd));
  }
  return 0;
};

std::unordered_map<string,FILE*> RFileDescriptorMap;
int getFileSize(string name) {
  auto search = RFileDescriptorMap.find(name);
  FILE* fd = NULL;
  bool isMapped = false;
  if (search != RFileDescriptorMap.end()) {
    fd = search->second;
    isMapped = true;
  } else {
    // Open the file -- Create if not present
    fd = std::fopen(name.c_str(), "rb");
    if (fd == NULL)
      return -1;
    RFileDescriptorMap.insert(std::make_pair(name , fd));
  }
  struct stat fileStat;
  fstat(fileno(fd) , &fileStat);
  return fileStat.st_size;
}

int getPacketFile (string name,char *data, int start, long offset, bool isLast) {
  using namespace std;
  int retVal = 0;
  auto search = RFileDescriptorMap.find(name);
  FILE* fd = NULL;
  bool isMapped = false;
  if (search != RFileDescriptorMap.end()) {
    fd = search->second;
    isMapped = true;
  } else {
    // Open the file -- Create if not present
    fd = std::fopen(name.c_str(), "rb");
    RFileDescriptorMap.insert(std::make_pair(name , fd));
  }
  if (start != -1 || ftell(fd) != start) {
    fseek(fd , start,SEEK_SET);
  };
  if (!feof(fd)) {
    fread(data,sizeof(char),offset, fd);
    // File writing done
    //std::cout << "File writing done " << std::endl;
  } 
  // If feof now 
  if (feof(fd)) {
    // std::cout << "EOF" << data << std::endl;
    // std::cout << "********" << std::endl;
    // EOF
    retVal = -1;
  }
  if (isLast) {
    fclose(fd);
    // Delete from map
    RFileDescriptorMap.erase(name);
  }
  return retVal;
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

    // Send file stat
    File_stats *ret = new File_stats();
    ret->fpath = fpath; ret->buffer_size = buffer_size;
    ret->start = start; ret->offset = offset;
    ret->totalSize = fileSize;
    ret->isFstat = true;
    std::unique_ptr<func_args> toCb1(new func_args());
    toCb1->forw = forw;    
    toCb1->func = ret;
    cb(std::move(toCb1));

    long chunk_size = offset - start;
    long readSize = 1;
    int seq = 0;    
    long goback;
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
        goback = cb(std::move(toCb));
        if (goback >= 0) {
          // Then go to that position           
          fseek(fd ,goback * buffer_size ,SEEK_SET);
          seq = goback;
          // Surely some bug here - but chunk_size is not used 
          chunk_size = goback * buffer_size;
          continue;
        }
        seq++;
        memset(buffer, 0 , sizeof(buffer));
        chunk_size -= readSize;
      } else {
        ret->eof = true;
        toCb->func = ret;
        // Pass eof 
        goback = cb(move(toCb));
        std::cout << "Going back " << goback << std::endl;
        if (goback >= 0 && goback < seq) {
          // Then go to that position           
          fseek(fd ,goback * buffer_size ,SEEK_SET);
          seq = goback;
          chunk_size = goback * buffer_size;
          continue;
        }
        // Reached end of file
        break;
      }
    }     
  };  
  fclose(fd);
  return 0;
}

#endif
