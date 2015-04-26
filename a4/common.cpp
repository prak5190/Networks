#ifndef __COMMON__
#define __COMMON__ 1 
#include <iostream>
#include <memory>
#include <cstdio>
#include <iomanip> 
#include <cstdlib>
#include <cstring>
#include <stdarg.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/epoll.h> 
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h> //icmp header
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h> 
#include <cstdarg>
#include <error.h>
#include <cmath>
#include <openssl/sha.h> //hashing pieces
#include <time.h>
#include <sys/time.h>
#include <bitset>
#include <unordered_map>
#include <queue>
#include <set>
// For sockaddr_in
#include <netinet/in.h>
// For port - htons
#include <arpa/inet.h>
// For gethostbyname
#include <netdb.h>
#include <sstream>
#include <string.h>
#include <pthread.h>
//0-10
float LOG_LEVEL = 5;
bool log_if(float level) {
  return level >= LOG_LEVEL ? true : false;
}
#include "timeutil.cpp"

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

using std::string;
using std::vector;
// #include "timeutil.cpp"
#include "bt_lib.cpp"
#include "fileHandler.cpp" 
#include "socketHelpers.cpp"
#include "bencode.cpp"
#include "msgHandler.cpp"
#endif

