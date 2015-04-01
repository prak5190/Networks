#include "common.cpp"
#include "udphandler.cpp"
using std::cout;
using std::endl;

void initApp(appglobals *app) {
  int s = initSocket();
  app->socket = s;
  app->mut     = PTHREAD_MUTEX_INITIALIZER;
  app->cond   = PTHREAD_COND_INITIALIZER;
  app->isDupElseSel = true;
  app->max_window_size = 100;  
  app->hasLatency = false;
  app->hasDrops = false;
  app->latencyTime = 20;
  app->drop_probability = 0.3;
};

int main (int argc , char** argv) {
  int port ;
  int destport;
  bool isSender , isReceiver;
  // Init socket
  pthread_t sthread, rthread;
  int senderThread , recThread;
  thread_args *args = new thread_args();  
  int c,i;
  initApp(&app);
  while ((c = getopt (argc, argv, "srW:D:L:")) != -1) {
    switch(c) {
    case 's' : isSender = true;
      break;
    case 'r' : 
      std::cout << "IS recieve_port" << std::endl;
      isReceiver = true;
      break;
    case 'W' :
      app.max_window_size = atoi(optarg);
      break;
    case 'D' : 
      app.hasLatency = true;
      app.latencyTime = atoi(optarg);
      break;
    case 'L' : 
      app.hasLatency = true;
      app.latencyTime = atoi(optarg);
      break;
    case '?':
      cout<<"Enter required info if any of the flags selected -> L , D , W "<<endl;
      break;
    default:
      std::cout << "Enter info " << optopt << std::endl;
    };      
  }
  
  if (isReceiver) {
    // Means reciever
    port = atoi(argv[argc-1]);    
    app.recieve_port = port;
    if ((recThread = pthread_create(&rthread , NULL ,createReciever, args)) == 0) {
      cout<<"Server created listening at "<< port <<endl;
    };
    // Join all required threads 
    if (recThread == 0)
      pthread_join(rthread,NULL);
  } else if (isSender) {    
    // Use port as 0 - means bind to any port 
    port = 0;
    app.recieve_port = port;
    if ((recThread = pthread_create(&rthread , NULL ,createReciever, args)) == 0) {
      std::cout << "Listner initialised "  << std::endl;
    };
       
    string fname = string(argv[argc-3]);
    destport = atoi(argv[argc-1]);
    cout<<"Destination port " << destport << endl;
    //Create a thread for the reciever
    args->port = destport;
    args->ip = string(argv[argc-2]);
    args->filename = fname;
    if ((senderThread = pthread_create(&sthread , NULL , getDataInThread, args)) == 0) {
      cout<<"Sender Thread created"<<endl;      
    };

    // Join all required threads 
    if (recThread == 0)
      pthread_join(rthread,NULL);
    if (senderThread == 0)
      pthread_join(sthread,NULL);
  } else {
      cout<<"Enter -s for sender , -r for receiver, -W to set maxwindow , -D to set drop probability and -L to set latency " <<endl;
  }
  return 0;
}

