#include "common.cpp"
struct thread_args {
 // Sending port
  bt_args_t *bt_args;
  int s;
};

long getRemainingSize(bt_args_t *bt_args) {
  long fileSize = bt_args->bt_info->length;
  long piece_length = bt_args->bt_info->piece_length;
  int num_pieces = bt_args->bt_info->num_pieces;
  // Iterate through pieces to get remaining sockets
  long rem = 0;
  for (auto it = piece_to_socket_map.begin(); it != piece_to_socket_map.end(); ++it) {
    int ind = it->first;
    if (ind < num_pieces - 1) {
      rem += piece_length;
    } else if (ind == num_pieces - 1) {
      rem += fileSize % piece_length;
    }
  }
  return fileSize - rem;
}
int getRemainingPieces(bt_args_t *bt_args) {
  int num_pieces = bt_args->bt_info->num_pieces;
  return num_pieces - completed_piece_to_socket_map.size();
}
// -1 represent file is completed
int getRandomPieceToDownload() {
  srandom(clock());
  int size = piece_to_socket_map.size();
  int rej = -1;
  if (size > 0) {
    int r = ((int) random()) % size;
    for (auto it = piece_to_socket_map.begin(); it != piece_to_socket_map.end(); ++it) {
      int ind = it->first;
      int s = it->second;
      if (s < 0 && r > ind) {
        return ind; 
      } else if (s < 0) {
        rej = ind;
      }
    }
  }
  return rej;
}
void setPieceAvailability(bt_args_t *bt_args,bt_msg_t *msg,int s) {
  int size = msg->payload.bitfiled.size;
  char *bits = msg->payload.bitfiled.bitfield;
  int totalPieces = bt_args->bt_info->num_pieces;
  int p = 0;
  for (int i = 0; i < size ; i++) {
    unsigned char a = bits[i];
    // Check every bit
    for (int j = 0; j < 8 ; j++) {
      if (p < totalPieces) {
        if (((a >> (7 - j)) & 1) == 1) {
          std::cout << "R: Registering piece " << p << std::endl;
          registerPiece(s,p);
        }
      } else {
        break;
      }
      p++;
    }
    if (p >= totalPieces) 
      break;
  }
}
void sendRequestForPieces(bt_args_t *bt_args,int s) {
  auto it1 = socket_to_piecelist_map.find(s);
  int requ = 0;
  if (piece_to_socket_map.size() == 0) {
    std::cout << "S: Am a seeder - No need to send a request " << std::endl;
    return;
  }
  std::cout << "S: Pieces available in socket " << s << " : " << it1->second.size() << std::endl;
  for (auto it = it1->second.begin(); it != it1->second.end(); ++it) {
    int length = 0;
    int piece = *it;
    std::cout << "S: Piece Available is " << piece << std::endl;
    // If piece is not in file - it will exist in map
    if (piece_to_socket_map.find(piece) != piece_to_socket_map.end()){
      requ++;
      // Start request 
      char *msg = createRequestMessage(bt_args,length,piece,(1 << 15) , 0);
      bt_msg_t kk; 
      memcpy((char*)&kk , msg , sizeof(kk));
      int n = send(s,msg,length,0);
      piece_to_socket_map.insert(std::make_pair(piece,s));
      if (n > 0){
        std::cout << "S: Sending request *********************"<< s << "    " << kk.bt_type << std::endl;        
      } else {
        std::cout << "SE: Failed " << strerror(errno) << std::endl;
      }
    }
  }
  if (requ == 0) 
    std::cout << "S: No Useful data found from socket "<< s << std::endl;
}
int handleData(bt_args_t *bt_args,const char* buf , int s) {
  int cbType = 0;
  if (buf[0] == 19) {
    cbType = 0;
  } else {             
    cbType = 1;     
  }
  switch(cbType) {
  case 0 : {
    // Handshake message - Lets parse it 
    handshake_msg_t hmsg;
    hmsg.parse(buf);
    std::cout << "R: Peer Id " <<  hmsg.peerId << std::endl;
  }break;
  case 1: {
    bt_msg_t msg;
    memset((char*)&msg,0x00,sizeof(msg));
    memcpy((char*)&msg , buf , sizeof(bt_msg_t));
    if (log_if(4.2)) {
      std::cout << "\nR: Data - Type : " << msg.bt_type << " - Buffer " << buf << std::endl; 
    }    
    // Now use the type to parse message 
    switch(msg.bt_type) {
    case BT_BITFILED: {
      bt_msg_t *m1 = parseBitField(buf, msg.length);
      if (log_if(4.3))
        std::cout << "R: Bitfield message "<< m1->payload.bitfiled.bitfield << std::endl;
      // Use the bitfield to assign a role to the Socket
      setPieceAvailability(bt_args,m1,s);      
    }break;
    case BT_REQUEST: {
      // msg already has reqyuired data - no need to reparse      
      std::cout << "R: Got a request "  << std::endl;
    }break;
    case BT_PIECE: {
      bt_msg_t *m1 = parsePieceMessage(buf, msg.length);
      if (log_if(4.4))
        std::cout << "R: Piece message "<< m1->payload.piece.piece << std::endl;
    }break;
    }
  }break;  
  default: {
    std::cout << "R: Buffer Overflow - SOme error  " << std::endl;
    // sleep(1);
    // Do something with the data    
  }break;
  }
  return 0;
}

int sendHandshakeMsg(bt_args_t *bt_args, int s) {
  handshake_msg_t msg;
  msg.setData(bt_args->bt_info->info_hash,string("dasda11111s"));     
  string message = msg.toString();
  std::cout << "SOCKET " << s << std::endl;
  int n = send(s,message.c_str(),message.length(),0);
  if (n >= 0) {
    registerSocket(s);
  }
  return n;
}

int sendBitFieldMessage(bt_args_t *bt_args) {
  int length = 0;
  char* m = createBitfieldMessage(bt_args,length);    
  // Send data to all   
  for (auto it = socket_to_piecelist_map.begin(); it != socket_to_piecelist_map.end(); ++it) {
    int s = it->first;
    int n = send(s,m,sizeof(bt_msg_t) + length,0); 
    //bt_msg_t *mgs = parseBitField(m,length);
    if (n > 0) {
      std::cout << "S: Message sent "<< s << std::endl;
    }
  }
  return 0;
}

// void findNewPeers() {
//   int port = INIT_PORT;
//   string ip = "localhost";
//   for (int i = port ; i < MAX_PORT; i++) {
//     if (i != ownPort) {
//       string key = ip + ":" + std::to_string(i);
//       auto it = url_to_socket_map.find(key);
//       std::cout << "S: Finding Key " << key << std::endl;
//       if (it == url_to_socket_map.end()) {
//         int sockfd;
//         struct sockaddr_in serv_addr;
//         sockfd = socket(AF_INET, SOCK_STREAM, 0);
//         if (sockfd < 0) {
//           perror("S: ERROR opening socket");
//           exit(1);
//         };
//         if (port != bt_args->port) {
//           bzero((char *) &serv_addr, sizeof(serv_addr));  
//           serv_addr.sin_family = AF_INET;
//           bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
//           serv_addr.sin_port = port;
//           /* Now connect to the server */
//           if (connect(sockfd,(struct sockaddr *) &serv_addr ,sizeof(serv_addr)) < 0) {
//             // std::cout << "Error connecting "<< strerror(errno) << std::endl;
//             // Close Socket 
//             // Try this after a few tries
//             //url_to_socket_map.insert(std::make_pair(key,-1000));
//             shutdown(sockfd,2);
//           } else {
//             url_to_socket_map.insert(std::make_pair(key,sockfd));
//             int n = sendHandshakeMsg(bt_args,sockfd);
//             if (n >= 0) {
//               std::cout << "S: Sent succesfully" << std::endl;                           
//             }
//           }
//         }
//       }
//     }
//   }
// }

void createSockAddr(int port,sockaddr_in *serv_addr) {
  struct hostent *server;
  string ip = "localhost";
  server = gethostbyname("localhost");
  bzero((char *) serv_addr, sizeof(sockaddr_in));  
  serv_addr->sin_family = AF_INET;
  serv_addr->sin_port = htons(port);
  serv_addr->sin_addr.s_addr = htonl(2130706433);
  //bcopy((char *)server->h_addr, (char *)serv_addr->sin_addr.s_addr, server->h_length);
}
void* initHandshake (void* args) {
  thread_args *t = (thread_args*) args;
  int sfd = t->s; 
  bt_args_t *bt_args = t->bt_args;  
  int ownPort = bt_args->port;
  int port = INIT_PORT;
  int efd = epoll_create1(0);
  if (efd == -1) {
    std::cout << "SRE: Fatal Epoll create error " << std::endl;
    abort();
  }
  //sleep(2);
  std::cout << "S: Start Init Handshake" << std::endl;
  for (port = INIT_PORT; port < MAX_PORT ; port++) {
    if (port != ownPort) {
      sockaddr_in serv_addr;
      createSockAddr(port,&serv_addr);
      //create and connect as much as needed        
      if (create_and_connect(&serv_addr, efd) != 0) {
        std::cout << "SE: Create and connect Error " << std::endl;
        exit(1);
      }
    }
  }   
  epoll_event event_mask;
  epoll_event events[MAXEVENTS];
  // event.data.fd = sfd;
  // event.events = EPOLLOUT | EPOLLET;
  // int s = epoll_ctl(efd,EPOLL_CTL_ADD,sfd,&event);
  // if (s == -1) {
  //   std::cout << "SRE: Fatal epoll CTL error " << std::endl;
  //   abort();
  // }  

  int n,i;
  char buf[BUFFER_SIZE];
  while(1) {
    n = epoll_wait (efd,events,MAXEVENTS,1000);
    //if (n > 0)  
      std::cout << "S: Got "<< n << " events for "<< sfd << std::endl;
    for (i=0;i<n;i++) {
      if ((events[i].events & EPOLLERR) ||
          (events[i].events & EPOLLHUP)) {
        //std::cout << events[i].events << std::endl;
        //std::cout << "SRE : Error receiving " << std::endl;
      } else { 
        // verify the socket is connected and doesn't return an error
        if(socket_check(events[i].data.fd) != 0) {
          std::cout << "SE : Write Handshake error " << std::endl;
          continue;
        } else {
          int dc = sendHandshakeMsg(bt_args,events[i].data.fd);
          if (dc < 0) {
            std::cout << "SE: Handshake send failed" << std::endl;
            abort();
          } else {
            std::cout << "SE : Hanshake Success " << std::endl;
            event_mask.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET;
            event_mask.data.fd = events[i].data.fd;                        
            if(epoll_ctl(receiver_efd, EPOLL_CTL_ADD, events[i].data.fd, &event_mask) != 0) {
              perror("epoll_ctl, modify socket\n");
              exit(1);
            }
          }
        }       
      }
    }
  }  
  // std::cout << "S: Found "<< size << " peers" << std::endl;
  // sendBitFieldMessage(bt_args);
  return 0;
}

void* createReciever (void* args) {
  thread_args *t = (thread_args*) args;
  int sockfd = t->s;  
  bt_args_t *bt_args = t->bt_args;
  std::cout << "Polling " << sockfd << std::endl;  
  while (1) {
    __npoll__(sockfd,handleData,bt_args);
    // Pass this data to handler thread
    //__poll__(sockfd,handleData,t->bt_args);    
  }
  std::cout << "Polling finished " << std::endl;
  return 0;
}
pthread_t startRecieverThread(bt_args_t *bt_args,int s) {  
  pthread_t rthread;
  int recThread;
  thread_args *args = new thread_args();  
  args->s = s;
  args->bt_args = bt_args;
  if ((recThread = pthread_create(&rthread , NULL ,createReciever, args)) == 0) {
    if (log_if(4.2))
      std::cout << "R: Started Reciever thread " << std::endl;    
  }
  if (recThread == 0) {
    return rthread;
  } else {
    return 0;
  }
}

pthread_t startSenderThread(bt_args_t *bt_args,int s) {
  pthread_t sthread;
  int sendThread;
  thread_args *t = new thread_args();
  t->s = s;
  t->bt_args = bt_args;
  if ((sendThread = pthread_create(&sthread , NULL ,initHandshake,t)) == 0) { 
    if (log_if(4.2))
      std::cout << "S: Init Handshake thread " << std::endl;    
  }
  if (sendThread == 0)
    return sthread;
  else 
    return 0;
}


struct ThreadState {
  pthread_mutex_t mut     = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  cond   = PTHREAD_COND_INITIALIZER;
  int waitTime(long ms) {
    // RTT in ms , so lets take some margin
    ms  += 10;
    return thread_timed_wait(this->mut,this->cond,ms);
  }
  void wait() {
    thread_wait(this->mut , this->cond);
  }
  void resume() {
    thread_resume(this->mut, this->cond);
  }
} HThreadState;

void* handlerThread(void* args) {
  sleep(1000);
  thread_args *t = (thread_args*) args;
  int sfd = t->s; 
  bt_args_t *bt_args = t->bt_args;
  //__spoll__(sfd , bt_args);
  return 0;;
};
pthread_t startHandlerThread(bt_args_t *bt_args,int s) {

  pthread_t sthread;
  int sendThread;
  thread_args *t = new thread_args();
  t->s = s;
  t->bt_args = bt_args;
  if ((sendThread = pthread_create(&sthread , NULL ,handlerThread,t)) == 0) { 
    if (log_if(4.2))
      std::cout << "S: Handler thread " << std::endl;
  }  
  if (sendThread == 0)
    return sthread;
  else 
    return 0;
}
