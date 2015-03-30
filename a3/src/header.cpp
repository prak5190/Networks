#include <inttypes.h>
#include "common.cpp"
#include <string.h>
#include <string>
#ifndef __HEADER_CPP__
#define __HEADER_CPP__ 1 
using std::string;
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
  // char hash[16];
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
  bool isDupElseSel = true;  
};
// Used in case of thread or in case of callback
struct func_args {
  void* func;
  void* forw;
};
struct thread_args {
  // Sending port
  int port;
  string ip;
  string filename;
};
#define PACKET_SIZE 1500
appglobals app;
long windowCounter;
// Socket helpers !!!!!!!!!!!!
int bindUdpSocket(int port , int s) {
  sockaddr_in in_addr;	   /* Structure used for bind() */
  in_addr.sin_family = AF_INET;                   /* Protocol domain */
  in_addr.sin_addr.s_addr = 0;                    /* Use wildcard IP address */  
  in_addr.sin_port = port;	       	   /* Use this UDP port */
  return bind(s, (struct sockaddr *)&in_addr, sizeof(in_addr));
}

sockaddr_in getSocketAddr(string ip , int port) {
  struct hostent *server;
  server = gethostbyname(ip.c_str());
  int socket = app.socket;
  sockaddr_in serv_addr;
  bzero((char *) &serv_addr, sizeof(sockaddr_in));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = port;
  return serv_addr;
}

// Reads data into udp_header 
void readPacket(char* buffer , udp_header *header , char* data) {
  if (buffer == NULL)
    return;
  size_t sz = sizeof(udp_header);
  memcpy(header , buffer , sz);
  memcpy(data , &buffer[sz]  , PACKET_SIZE - sz);
  // if (&buffer[sz] != NULL) {
  //   data = new char[strlen(&buffer[sz])];
  //   strcpy(data,&buffer[sz]);
  //   std::cout << "data " << data << std::endl;
  // }
};

fileInfo* getFileInfoFromData(char* data) {
  fileInfo* f = new fileInfo();
  memcpy(f , data , sizeof(fileInfo));  
  return f;
}
void createRequestPacket(char* buffer, udp_header *header, fileInfo *finfo) {
  size_t sz = sizeof(udp_header);
  memcpy(buffer, header, sz);
  memcpy(&buffer[sz] , finfo , sizeof(fileInfo));
};

void writeToBuffer(char* buffer, udp_header *header, const char* data) {
  size_t sz = sizeof(udp_header);
  memcpy(buffer, header, sz);
  memcpy(&buffer[sz] , data , PACKET_SIZE - sz);
};

// Pthread stuff
void thread_resume() {
  pthread_mutex_t mut = app.mut;
  pthread_cond_t cond = app.cond;
  pthread_mutex_lock(&mut);
  pthread_cond_broadcast(&cond);
  // pthread_cond_wait(&cond, &mut);
  pthread_mutex_unlock(&mut);  
};
void thread_wait() {
  pthread_mutex_t mut = app.mut;
  pthread_cond_t cond = app.cond;
  pthread_mutex_lock(&mut);
  pthread_cond_wait(&cond, &mut);
  pthread_mutex_unlock(&mut);  
}
void thread_timed_wait (int mseconds) {
  pthread_mutex_t mut = app.mut;
  pthread_cond_t cond = app.cond;
  timespec time;
  clock_gettime(CLOCK_REALTIME, &time);  
  time.tv_nsec += 1000 * mseconds;
  pthread_mutex_lock(&mut);
  pthread_cond_timedwait(&cond, &mut,&time);
}
// All calculations in millis
long calculateRTT(long sampleRtt , long estRtt , long deviation) {
  // 3 is bacase of sigma = 1/8 - i.e unshift by 3
  sampleRtt -= (estRtt >> 3) ;
  estRtt += sampleRtt; 
  if (sampleRtt < 0)
    sampleRtt = -sampleRtt;
  sampleRtt -= (deviation >> 3);
  deviation += sampleRtt;
  // Timout
  return (estRtt >> 3) + (deviation >> 1);
}
#endif


