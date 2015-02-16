#include "common.cpp"
#include <unistd.h>
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

// Creating server 
int create_server() {
  int sfd = socket(AF_INET , SOCK_DGRAM , 0);
  if (sfd == -1)                
    error("socket err");
  // struct sockaddr_in *my_addr, *peer_addr;
  /* Initialize socket structure */
  struct sockaddr_in serv_addr;  
  struct sockaddr_in clientaddr; 
  char buf[1255]; /* message buf */
  int newsockfd;

  bzero((char *) &serv_addr, sizeof(serv_addr));   
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(2222);

  if(bind (sfd , (struct sockaddr*) &serv_addr , sizeof(serv_addr)) < 0)
    error("Unable to bind \n");
  else 
    cout<<"Binding complete to 2222 \n";

  while(1) {
    pthread_t thread;
    thread_args *args = new thread_args();
    bzero((char *) args, sizeof(thread_args));

    socklen_t clientlen = (socklen_t)sizeof(clientaddr);

    cout<<"Waiting "<<endl;    

    int n = recvfrom(sfd, buf, 1000, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    cout<<"Recieved message \n"<<buf<<endl;    
    n = sendto(sfd, buf, strlen(buf), 0, 
	       (struct sockaddr *) &clientaddr, clientlen);
  };
  
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
      char *str = readFromSocket(newsockfd,n);
      cout<<"********************************** " <<n;

      if (n < 0) {
        error("ERROR reading from socket");            
        break;
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



// Creating client
int main (int argc ,char** argv) {  
  create_server();
  return 0;
}
