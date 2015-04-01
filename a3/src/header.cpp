#include <inttypes.h>
#include "common.cpp"
#include <string.h>
#include <string>
#ifndef __HEADER_CPP__
#define __HEADER_CPP__ 1 
using std::string;
enum class AckCodes : uint16_t {
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
#endif


