#include <inttypes.h>
#include "common.cpp"
#include <string.h>
#include <string>
#ifndef __HEADER_CPP__
#define __HEADER_CPP__ 1 
using namespace std;
/** Header struct , parsing and setting **/
struct udp_header {
  uint16_t ttl;  
  // // Total length 
  // uint64 length;

  // sequence and ack are of same length - TCP has 32 bit header so following it 
  // In reality even 16 bit is ok
  // sequnece number and ack are limited by window size - manually by the program
  uint32_t seq;  
  uint32_t ack;
  // Md5 hash of file - Any file sent has its md5 hash stored in a hashmap at server and client side to denote file info
  // This hash is used to identify the file while downloading 
  char hash[16];
  // This indicates if the data only contains file info - When true the data will contain all the info about the hash in the 
  // header - This currently just means file name and size
  bool hasFileInfo;
  bool isRequest;
};

struct fileInfo { 
  string filename;
  uint64_t size;
};

struct appglobals {
  int socket;
  // This mutex tells the sender to wait when window is filled
  pthread_mutex_t mut     = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  cond   = PTHREAD_COND_INITIALIZER;
  long window_size;
  int recieve_port;
};
struct thread_args {
  appglobals* app;
  // Sending port
  int port;
};

#endif


