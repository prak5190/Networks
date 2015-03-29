#include "common.cpp"
#include "recieve.cpp"

int initSocket();
void initApp(appglobals *app , int port) {
  int s = initSocket();
  // Stop and wait
  int window_size = 1;
  app->socket = s;
  app->recieve_port = port;
  app->window_size = window_size;
};

int main (int argc , char** argv) {
  int port ;
  int destport;

  // Init socket
  appglobals app;
  pthread_t sthread, rthread;
  int senderThread , recThread;
  thread_args *args = new thread_args();
  bzero((char *) args, sizeof(thread_args));
  if (argc > 1) {
    port = atoi(argv[1]);    
    initApp(&app,port);
    args->app = &app;
    if (argc > 2) {
      destport = atoi(argv[2]);
      cout<<"Destination port " << destport;
      // Create a thread for the recieve
      args->port = destport;
      if ((senderThread = pthread_create(&sthread , NULL ,sendDataInThread, args)) == 0) {
        cout<<"Sender Thread created";
      };
    }
    if ((recThread = pthread_create(&rthread , NULL ,createReciever, args)) == 0) {
      cout<<"Sender Thread created";
    };    
    // Join all required threads 
    if (senderThread == 0)
      pthread_join(sthread,NULL);
    if (recThread == 0)
      pthread_join(rthread,NULL);
  };  
  return 0;
}






int initSocket() {
  // Create socket 
  int s;			   /* s = socket */
  s = socket(AF_INET, SOCK_DGRAM, 0);
  //  Enable reuse
  {  
    int true1 = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &true1,
                   sizeof(true1)) == -1)
      { perror("reuseaddr");
        return -1;
      }
  }
  return s;
};
