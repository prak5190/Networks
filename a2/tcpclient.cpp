#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "common.cpp"
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

// Creating client
int client (int portno , const char* host , const char* path ) {
  //create_server
  int sockfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  
  char buffer[256];
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
  };
  
  server = gethostbyname(host);

  bzero((char *) &serv_addr, sizeof(serv_addr));  
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  /* Now connect to the server */
  if (connect(sockfd,(struct sockaddr *) &serv_addr ,sizeof(serv_addr)) < 0) {
    error("ERROR connecting");
  }
  
  const char* msg = "GET %s HTTP/%s \r\n Host: %s \r\n Connection: keep-alive\r\n User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/34.0.1847.116 Chrome/34.0.1847.116 Safari/537.36\r\n\r\n Accept-Encoding: gzip,deflate,sdch \n\n";

  char message[1000];
  bzero(message , sizeof(message));
  char protocol[] = "1.0";
  sprintf(message,msg ,path, protocol , host);
  /* Send message to the server */
  n = send(sockfd,message,strlen(msg),0);
   
  if (n < 0)    {
    error("ERROR writing to socket");
  }
   
  /* Now read server response until length*/
  char *str = readFromSocket(sockfd,n);
  // bzero(buffer,256);
  // n = read(sockfd,buffer,255);   
  if (n < 0)    {
    error("ERROR reading from socket");
  }

  printf("%s\n",str);
  close(sockfd); 
  return 0;
}
static bool keepRunning = true;

void shutdown(int dummy=0) {
    keepRunning = false;
    cout<<endl<<"Shutting down server";
    cout.flush();
    exit(1);
}

int print(const char* str) {
  cout<<str;
  cout.flush();
  return 0;
}


// Creating client
int main (int argc ,char** argv) {  
  int port = 1120 , tmp,c ;  
  char* host , *path ,*tmp2;  
  int n;
  while ((c = getopt (argc, argv, "h:f:p:")) != -1) {
    switch(c) {
    case 'p' : 
      tmp = atoi(optarg);
      if(tmp != 0) {
        port = tmp;
        cout<<"Binding port to "<<port << endl;
      }
      break;
    case 'h' : 
      tmp2 = optarg;
      n = sizeof(optarg);
      host = new char [n];
      strcpy(host , optarg);
      cout<<"Host is :  "<< host << endl;
      break;
    case 'f' : 
      tmp2 = optarg;
      n = sizeof(optarg);
      path = new char [n];
      strcpy(path , optarg);
      cout<<"File is :  "<< path << endl;
      break;
    case '?':
      if (optopt == 'p')
        cout<<"Enter port info";
      break;
    default:
      cout<<"Option is "<<c <<endl;
    };      
  };
  
  signal(SIGINT, shutdown);
  client(port , host , path);
  return 0;
}
