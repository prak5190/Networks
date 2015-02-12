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

struct thread_args {
  int sfd;
};

void *handle_connection(void *t);

int writeToClient(const char* buf , int fd) {
  write(fd,buf,strlen(buf));
  return 0;
};

// Creating server 
int sfd = socket(AF_INET , SOCK_DGRAM , 0);

int shutdown_tcp(){
  if(shutdown(sfd,2) == 0){
    cout<<endl<<"Succesful Shutdown ....." << endl;
    cout.flush();
  } else{
    cout<<endl<<"Unable to Shutdown ....." << endl;
    cout.flush();   
  };
  return 0;
};


int create_server() {
  if (sfd == -1)                
    error("socket err");

  // struct sockaddr_in *my_addr, *peer_addr;
  /* Initialize socket structure */
  struct sockaddr_in serv_addr;  
  bzero((char *) &serv_addr, sizeof(serv_addr));
   
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(2222);

  if(bind (sfd , (struct sockaddr*) &serv_addr , sizeof(serv_addr)) < 0)
    error("Unable to bind \n");
  else 
    cout<<"Binding complete to 2222 \n";

  while(1) {
    listen(sfd,5); 
    pthread_t thread;
    thread_args *args = new thread_args();
    bzero((char *) args, sizeof(thread_args));

    struct sockaddr_in cli_addr;
    socklen_t clilen = (socklen_t)sizeof(cli_addr);

    int newsockfd = accept(sfd, (struct sockaddr *)&cli_addr, &clilen);

    args->sfd = newsockfd;
    if(pthread_create(&thread , NULL , handle_connection, args)){
      cout<<"Thread created";
    };
    //handle_connection(sfd);
  };
  
  //  exit(1);
  return 0;
};






void* handle_connection(void *args) {
  thread_args *t = (thread_args*)args;
  int newsockfd = t->sfd;
  char buffer[256];
  /* Accept actual connection from the client */
  if (newsockfd < 0) {
    error("ERROR on accept");    
  } else {
    bool first = true ;
    while(1) {
      // This is an echo server
      /* If connection is established then start communicating */
      int n;
      char *str = new char[1];
      do {
        bzero(buffer,256);    
        n = read(newsockfd,buffer,255);
        char *newstr = new char [strlen(str) + strlen(buffer)];    
        strcat(newstr ,str);
        str = strcat(newstr ,buffer);
        cout.flush();    
      } while (n > 0 && n == 255);

      if (n < 0) {
        error("ERROR reading from socket");    
        return;
      } else {
        if(first) {
          writeToClient("I am an echo server and am just gonna say wat you say",newsockfd);
        } else {
          writeToClient(str,newsockfd);
        }              
      } 
        
      if (n < 0) {
        error("ERROR writing to socket");
      }
    };

  }
      
       
  cout.flush();
  shutdown(newsockfd,2);
}
