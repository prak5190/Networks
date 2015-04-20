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
#include <unistd.h>
#include <fcntl.h>
#include <signal.h> 
#include <cstdarg>
#include <error.h>
#include <cmath>
#include <time.h>
#include <sys/time.h>
#include <bitset>
#include <unordered_map>
#include <map>
// For sockaddr_in
#include <netinet/in.h>
// For port - htons
#include <arpa/inet.h>
// For gethostbyname
#include <netdb.h>
#include <string.h>
#include <pthread.h>
//0-10
float LOG_LEVEL = 5;
bool log_if(float level) {
  return level >= LOG_LEVEL ? true : false;
}
using std::string;
// #include "timeutil.cpp"
#include "socketHelpers.cpp"
#include "fileHandler.cpp" 
#include "bt_lib.h"
#include "bencode.cpp"
#endif

