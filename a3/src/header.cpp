#include <inttypes.h>
#include "common.cpp"
#include <string.h>
#include <string>
#ifndef __HEADER_CPP__
#define __HEADER_CPP__ 1 
using std::string;
enum class AckCodes : int {
  RhasFileInfo,
  RData,
  ShasFileInfo,
  SduplicateAck,
  SrepeatAck,
  SsuccessAck,
  SwrapSequence
};

/** Header struct , parsing and setting **/
struct udp_header {
  uint16_t ttl;  
  uint16_t window_size;
  // // Total length 
  // uint64 length;
  // sequence and ack are of same length - TCP has 32 bit header so following it 
  // In reality even 16 bit is ok
  // sequnece number and ack are limited by window size - manually by the program
  uint32_t seq;  
  //int32_t ack;
  AckCodes ack;
  // Md5 hash of file - Any file sent has its md5 hash stored in a hashmap at server and client side to denote file info
  // This hash is used to identify the file while downloading 
  // char hash[16];
  // This indicates if the data only contains file info - When true the data will contain all the info about the hash in the 
  // header - This currently just means file name and size    
  bool hasFileInfo;
};

struct fileInfo { 
  char filename[100];
  uint64_t size;
};
// Pthread stuff
void thread_resume(pthread_mutex_t &mut,pthread_cond_t &cond) {
  pthread_mutex_lock(&mut);
  pthread_cond_broadcast(&cond);
  // pthread_cond_wait(&cond, &mut);
  pthread_mutex_unlock(&mut);  
};
int thread_wait(pthread_mutex_t &mut,pthread_cond_t &cond) {
  pthread_mutex_lock(&mut);
  int error = pthread_cond_wait(&cond, &mut);
  pthread_mutex_unlock(&mut);  
  return error;
};
int thread_timed_wait (pthread_mutex_t &mut ,pthread_cond_t &cond ,int mseconds) {
  timespec time;
  clock_gettime(CLOCK_REALTIME, &time);
  timespec_addms(&time,mseconds);
  pthread_mutex_lock(&mut);
  int error = pthread_cond_timedwait(&cond, &mut,&time);
  pthread_mutex_unlock(&mut);
  return error;
};

struct appglobals {
  int socket;
  // This mutex tells the sender to wait when window is filled
  pthread_mutex_t mut ; 
  pthread_cond_t  cond ;
  long max_window_size;
  int recieve_port;
  bool isDupElseSel;

  bool hasLatency, hasDrops;
  long latencyTime;
  float drop_probability;
};
struct SenderState {
  long windowSize, seqNum , expSeqNum = 999999999 , lastAckedNum, windowCounter;
  timespec  *lastMsgTime; 
  // Rtt in milliseconds
  long rtt = 0;
  bool isSending;
  // Address of last reciever 
  sockaddr_in  recv_addr;
  socklen_t recv_len;
  pthread_mutex_t mut     = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  cond   = PTHREAD_COND_INITIALIZER;
  void wait() {
    thread_wait(this->mut , this->cond);
  }
  void resume() {
    thread_resume(this->mut, this->cond);
  }
  int waitTime(long ms) {
    return thread_timed_wait(this->mut,this->cond,ms);
  }
  void setTime() {
    timespec* lastMsgTime = this->lastMsgTime;
    if (lastMsgTime == NULL) 
      lastMsgTime = new timespec();
    timespec_now(lastMsgTime);
    this->lastMsgTime = lastMsgTime;
  }
  void setRTT() {
    // Get prev time 
    timespec *prev = this->lastMsgTime;
    // Get current time 
    this->lastMsgTime = new timespec();    
    this->setTime();    
    long sampleRtt;
    if(prev != NULL) {
      timespec_subtract(this->lastMsgTime,prev);
      sampleRtt = timespec_milliseconds(this->lastMsgTime);
      delete prev;
    } else {
      this->setTime();
      return;
    }
    long estRtt = this->rtt;
    long deviation = sampleRtt - estRtt;
    if (estRtt == 0) {
      // Just set it to sample
      this->rtt = sampleRtt;
    } else {
      // Calcualate RTT 
      this->rtt =  calculateRTT(sampleRtt , estRtt , deviation);
    }
    this->setTime();
  }
  ~SenderState () {
    delete this->lastMsgTime;
  }
} senderState;
struct RecieverState {
  bool isRecieving;
  long windowSize, lastRecievedSeq , seqBase , seqMax,windowCounter , fileSize;
  timespec  *lastMsgTime; 
  long initialFileSize;
  string filename;
  // Rtt in milliseconds
  long rtt = 0;
  long totalPackets;
  bool fileClosed;
  // Address of last reciever 
  sockaddr_in  recv_addr;
  socklen_t recv_len;
  pthread_mutex_t mut    = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  cond   = PTHREAD_COND_INITIALIZER;
  void wait() {
    thread_wait(this->mut , this->cond);
  }
  void resume() {
    thread_resume(this->mut, this->cond);
  }
  int waitTime(int ms) {
    return thread_timed_wait(this->mut,this->cond,ms);
  }
  void setTime() {
    timespec* lastMsgTime = this->lastMsgTime;
    if (lastMsgTime == NULL) 
      lastMsgTime = new timespec();
    timespec_now(lastMsgTime);
    this->lastMsgTime = lastMsgTime;
  }
  void setRTT() {
    // Get prev time 
    timespec *prev = this->lastMsgTime;
    // Get current time 
    this->lastMsgTime = new timespec();    
    this->setTime();    
    long sampleRtt;
    if(prev != NULL) {
      timespec_subtract(this->lastMsgTime,prev);
      sampleRtt = timespec_milliseconds(this->lastMsgTime);
      delete prev;
    } else {
      this->setTime();
      return;
    }
    long estRtt = this->rtt;
    long deviation = sampleRtt - estRtt;
    if (estRtt == 0) {
      // Just set it to sample
      this->rtt = sampleRtt;
    } else {
      // Calcualate RTT 
      this->rtt =  calculateRTT(sampleRtt , estRtt , deviation);
    }
    this->setTime();
  }

  ~RecieverState () {
    delete this->lastMsgTime;
  }
} recieverState;
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
int DATA_SIZE = PACKET_SIZE - sizeof(udp_header);
appglobals app;
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
};
void readPacket(char* buffer , udp_header *header , fileInfo *data) {
  if (buffer == NULL)
    return;
  size_t sz = sizeof(udp_header);
  memcpy(header , buffer , sz);
  memcpy(data , &buffer[sz]  , sizeof(fileInfo));
};


void getFileInfoFromData(char* data, fileInfo *f) {
  memcpy(f, data , sizeof(fileInfo));  
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

// Print Utils 
float lastProgress;
void printRecProgress() {
  float progress = ((recieverState.initialFileSize - recieverState.fileSize)) * 100/ (recieverState.initialFileSize); 
  //std::cout << "Progress " << progress << std::endl;
  if (progress > lastProgress + 0.5) {
    if (((int)progress) % 10 == 0)
      std::printf("Progress %.0f \n", progress);
    lastProgress = progress;
  }
  //   std::printf("Progress %.2f \r", progress);
  // }
}

#endif


