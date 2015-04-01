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
bool log(float level) {
  return level >= LOG_LEVEL ? true : false;
}

// struct client_args {
//   bool isRec , isSender;
//   bool hasLatency , hasDrops;
//   long max_window_size;
//   long latencyTime;
//   float drop_probability;
//   string filename;
//   string ip;
//   int port;
// };

// client_args getClientArgs(int argc , char** argv){ 
//   char* host , *path ,*tmp2;  
//   int n,tmp,c,protocol;
//   int port = 9000; // Default port
//   client_args args ;
//   // last argument is always port - sender or reciever 
//   // 2nd last in case of 
//   while ((c = getopt (argc, argv, "rsl:d:f")) != -1) {
//     switch(c) {
//     case 'p' : 
//       tmp = atoi(optarg);
//       if(tmp != 0) {
//         port = tmp;
//         cout<<"Binding port to "<<port << endl;
//       }
//       break;
//     case 'r' : 
//       tmp = atoi(optarg);
//       protocol = tmp;
//       if(tmp != 0) {
//         protocol = 1;
//       } else {
//         protocol = 0;
//       }
//       cout<<"Protocol HTTP 1."<<protocol << endl;       
//       break;
//     case 'h' : 
//       tmp2 = optarg;
//       n = sizeof(optarg);
//       host = new char [n];
//       strcpy(host , optarg);
//       cout<<"Host is :  "<< host << endl;
//       break;
//     case 'f' : 
//       tmp2 = optarg;
//       n = sizeof(optarg);
//       path = new char [n];
//       strcpy(path , optarg);
//       cout<<"File is :  "<< path << endl;
//       break;
//     case 't':
//       args.isTime = true;
//       break;
//     case '?':
//       if (optopt == 'p')
//         cout<<"Enter port info";
//       break;
//     default:
//       cout<<"Option is "<<c <<endl;
//     };      
//   };
//   args.host = host ;
//   args.path = path;
//   args.port = port;
//   args.protocol = protocol;
//   return args;
// };


#include "timeutil.cpp"
#include "header.cpp"
#include "state.cpp"
#include "socketHelpers.cpp"
#include "fileHandler.cpp"
#endif

