#include "common.cpp"
#include "udphandler.cpp"
using std::cout;
using std::endl;
int initSocket();
void initApp(appglobals *app , int port) {
  int s = initSocket();
  app->socket = s;
  app->max_window_size = 100;
  app->recieve_port = port;
};

int main (int argc , char** argv) {
  int port ;
  int destport;
  // Init socket
  pthread_t sthread, rthread;
  int senderThread , recThread;
  thread_args *args = new thread_args();

  if (argc == 2) {
    // Means reciever
    port = atoi(argv[1]);    
    initApp(&app,port);

    if ((recThread = pthread_create(&rthread , NULL ,createReciever, args)) == 0) {
      cout<<"Server created listening at "<< port <<endl;
    };
    // Join all required threads 
    if (recThread == 0)
      pthread_join(rthread,NULL);
  } else if (argc == 3) {    
    // Use port as 0 - means bind to any port 
    port = 0;
    initApp(&app,port);
    if ((recThread = pthread_create(&rthread , NULL ,createReciever, args)) == 0) {
      std::cout << "Listner initialised "  << std::endl;
    };
       
    string fname = string(argv[1]);
    destport = atoi(argv[2]);
    cout<<"Destination port " << destport << endl;
    //Create a thread for the reciever
    args->port = destport;
    args->ip ="localhost";
    args->filename = fname;
    if ((senderThread = pthread_create(&sthread , NULL , getDataInThread, args)) == 0) {
      cout<<"Sender Thread created"<<endl;      
    };

    // Join all required threads 
    if (recThread == 0)
      pthread_join(rthread,NULL);
    if (senderThread == 0)
      pthread_join(sthread,NULL);
  }
  return 0;
}

int initSocket() {
  // Create socket 
  int s;			   /* s = socket */
  s = socket(AF_INET, SOCK_DGRAM, 0);
  //  Enable reuse
  int true1 = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &true1,
                 sizeof(true1)) == -1) {
    perror("reuseaddr");
    return -1;
  }
  // if (setsockopt(s,IPPROTO_IP,IP_RECVTTL , (char *) &true1,
  //                sizeof(true1)) == -1)
  //   { perror("RecvTTL error");
  //     return -1;
  //   }
  return s;
};
