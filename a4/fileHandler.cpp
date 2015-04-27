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
  if (search != FileDescriptorMap.end()) {
    fd = search->second;
    fclose(fd);
    // Delete from map
    FileDescriptorMap.erase(name);
  }
  return 0;
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

int createFile(string name,long size) {
  char data[1];
  data[0] = 'a';
  assembleFile (name,size-1,data,1,true);
  return 0;
};

std::unordered_map<string,FILE*> RFileDescriptorMap;
long getFileSize(string name) {
  auto search = RFileDescriptorMap.find(name);
  FILE* fd = NULL;
  if (search != RFileDescriptorMap.end()) {
    fd = search->second;
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
  if (search != RFileDescriptorMap.end()) {
    fd = search->second;
  } else {
    // Open the file -- Create if not present
    fd = std::fopen(name.c_str(), "rb");
    RFileDescriptorMap.insert(std::make_pair(name , fd));
  }
  if (start != -1 || ftell(fd) != start) {
    fseek(fd , start,SEEK_SET);
  };
  if (!feof(fd)) {
    //if (log_if(3.1))
    std::cout << "SIze " << offset << std::endl;
    retVal = fread(data,sizeof(char),offset, fd);
    std::cout << "Ret Val " << retVal << std::endl;    
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

#endif
