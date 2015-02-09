#include "common.cpp"
#include <unistd.h>
#include "tcpserver.cpp"
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/un.h>
// For sockaddr_in
#include <netinet/in.h>
// For port - htons
#include <arpa/inet.h>
// For gethostbyname
#include <netdb.h>

#define BUF_SIZE 100
using namespace std; 
int print(const char* str) {
  cout<<str;
  cout.flush();
  return 0;
}
// Creating client
int main (int argc ,char** argv) {  
  create_server();
  return 0;
}
