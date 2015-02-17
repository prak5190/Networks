#include "common.cpp"
#include <pthread.h>
// For sockaddr_in
#include <netinet/in.h>
// For port - htons
#include <arpa/inet.h>

#define BUF_SIZE 100
using namespace std; 

struct thread_args {
  int sfd;
};
static bool justDisplayTime = false;
// The broken pipe error
void signal_callback_handler(int signum){
  printf("Caught signal SIGPIPE %d\n",signum);
}

void *handle_connection(void *t);

// Creating server 
int sfd = socket(AF_INET , SOCK_STREAM , 0);

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


int create_server(int port) {
  if (sfd == -1)                
    error("socket err");

  // struct sockaddr_in *my_addr, *peer_addr;
  /* Initialize socket structure */
  struct sockaddr_in serv_addr;  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  if(bind (sfd , (struct sockaddr*) &serv_addr , sizeof(serv_addr)) < 0)
    error("Unable to bind");
  else 
    cout<<"Binding complete to port "<<port<<" \n";

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
  // Start time
  clock_t start, end;
  double cpu_time_used;
  start = clock();

  // cout<<"Handling connection";
  // cout.flush();
  /* Accept actual connection from the client */
  if (newsockfd < 0) {
    error("ERROR on accept");    
  }
      
  /* If connection is established then start communicating */
  char buffer[256];
  int n = 0;
  char *str = readFromSocketS(newsockfd,n);
  if (n < 0) {
    error("ERROR reading from socket");    
  } else {      
    // Parse the buffer to get http headers 
    struct http_headers *headers = parseHttpHeaders(str);
    // cout<<"Here is the message: \n"<<str;
    const char* path = headers->path;
    cout<<endl<<"The Path " << path<<endl;
    cout.flush();
    if (path && strcmp(path,"/") != 0) {
      char respath[] = "www/";
      strcat(respath,path);
      //cout<<endl<<"Thread processing file "<<respath;
      int m = getFile(respath , writeToClient , newsockfd);
      if (m < 0){     
        writeToClient("HTTP/1.0 404 Not Found\n Content-type: text/html \n\n <html><body><h2>Not found </h2></body></html>",newsockfd); 
      }
    } else if (!path || strcmp(path,"/") == 0) {
      /* Write a response to the client */
      writeToClient("Hi , Type a path to a file in the www folder if you know one :)",newsockfd);
    } else {
      writeToClient("HTTP/1.0 404 Not Found\n Content-type: text/html \n\n <html><body><h2>Not found </h2></body></html>",newsockfd);
    }
         
    if (n < 0) {
      error("ERROR writing to socket");
    }       
    close(newsockfd);
  }
  //cout<<"Writing finished ";
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  cout<<endl<<"Time taken "<<cpu_time_used<<endl;
  cout.flush();
  shutdown(newsockfd,2);
}

void shutdown(int dummy=0) {
    cout<<endl<<"Shutting down server";
    cout.flush();
    shutdown_tcp();
    exit(1);
}

int print(const char* str) {
  cout<<str;
  cout.flush();
  return 0;
}


// Creating client
int main (int argc ,char** argv) {  
  int c,port = 1120,tmp ;
  while ((c = getopt (argc, argv, "p:t")) != -1) {
    switch(c) {
    case 'p' : 
      tmp = atoi(optarg);
      if(tmp != 0) {
        port = tmp;
        cout<<"Binding port to "<<port << endl;
      }
      break;
    case 't':
      justDisplayTime = true;
    case '?':
      if (optopt == 'p')
        cout<<"Enter port info";
      break;
    default:
      cout<<"Option is "<<c <<endl;
    };
      
  };
  signal(SIGPIPE, signal_callback_handler);
  //signal(SIGSEGV, segv_handler);
  signal(SIGINT, shutdown);
  create_server(port);
  return 0;
}
