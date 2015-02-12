#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "common.cpp"
#include <unistd.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
// For sockaddr_in
#include <netinet/in.h>
// For port - htons
#include <arpa/inet.h>

#define BUF_SIZE 100
using namespace std; 
struct http_headers {
  const char *path,*method,*protocol;
};      

http_headers* parseHttpHeaders(char* buf) {
  http_headers* h = new http_headers();
  FILE *str = fmemopen(buf,strlen(buf),"r");
  // Reading first line to get path and method
  char *saveptr;
  size_t len = 0;
  int j = 0;
  // Can make this a while 
  {
    char *line = NULL ;
    getline(&line,&len,str);    
    cout<<"Line is " <<line;
    char *token = NULL ;
    int i = 0;
    do {      
      token = strtok_r(line, " :",&saveptr);      
      line = NULL;
      // Depending on i -- infer meaning 
      switch(j) {
      case 0:
        switch (i) {
        case 0 : h->method = token; break;
        case 1 : h->path = token; break;
        case 2 : h->protocol = token; break;
        };
      };
      i++;
    } while (token != NULL);

    j++;
  }
  // Parse the headers 
  //  h->path = "www/tcpserver";
  return h;
};

int writeToClient(const char* buf , int fd) {
  write(fd,buf,strlen(buf));
  return 0;
};

// Creating server 
int sfd = socket(AF_INET , SOCK_STREAM , 0);

int shutdown_tcp(){
  shutdown(sfd,2);
  return 0;
};
int create_server() {
  char buffer[256];
  if (sfd == -1)                
    error("socket err");

  // struct sockaddr_in *my_addr, *peer_addr;
   /* Initialize socket structure */
  struct sockaddr_in serv_addr, cli_addr;
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
   
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(1111);

  if(bind (sfd , (struct sockaddr*) &serv_addr , sizeof(serv_addr)) < 0)
    error("Unable to bind");
  else 
    cout<<"Binding complete to 1111 \n";

  while(1) {
      listen(sfd,5);
      int newsockfd;
      socklen_t clilen = (socklen_t)sizeof(cli_addr);
      /* Accept actual connection from the client */
      newsockfd = accept(sfd, (struct sockaddr *)&cli_addr, &clilen);

      if (newsockfd < 0) {
        error("ERROR on accept");    
      }
      
      /* If connection is established then start communicating */
      bzero(buffer,256);
      int n = read(newsockfd,buffer,255 );
   
      if (n < 0) {
        error("ERROR reading from socket");    
      }
  
      // Parse the buffer to get http headers 
      struct http_headers *headers = parseHttpHeaders(buffer);
      cout<<"Here is the message: \n"<<buffer;
      const char* path = headers->path;
      cout<<endl<<"The Path " << path<<endl;
      cout.flush();
      if (path && strcmp(path,"/") != 0) {
        char respath[] = "www/";
        strcat(respath,path);
        getFile(respath , writeToClient , newsockfd);
        writeToClient("Unable to open file ",newsockfd);      
      } else {
        /* Write a response to the client */
        writeToClient("Hi , Type a path to a file in the www folder if you know one :)",newsockfd);      
      }
      
   
      if (n < 0) {
        error("ERROR writing to socket");
      }
       
      cout.flush();
      shutdown(newsockfd,2);
  };
  
  //  exit(1);
  return 0;
};






