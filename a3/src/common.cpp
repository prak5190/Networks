#ifndef __COMMON__
#define __COMMON__ 1 
#include <iostream>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdarg.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h> 
#include <cstdarg>
#include <error.h>
#include <time.h>
#include <sys/time.h>
#include <bitset>
#include <unordered_map>
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
bool log(float level) {
  return level >= LOG_LEVEL ? true : false;
}

#include "timeutil.cpp"
#include "header.cpp"
#include "fileHandler.cpp"
#endif

