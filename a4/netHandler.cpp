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
  unsigned char *bits = msg->payload.bitfiled.bitfield;
  int totalPieces = bt_args->bt_info->num_pieces;
  int p = 0;
  for (int i = 0; i < size ; i++) { 
    unsigned char a = (unsigned char)bits[i];
    // Check every bit
    for (int j = 0; j < 8 ; j++) {
      if (p < totalPieces) {
        if (((a >> (7 - j)) & 1) == 1) {
          //std::cout << "R: Registering piece " << p << std::endl;
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

void writeToFile(bt_args_t *bt_args, bt_msg_t *msg,int s) {
  string fs = string(bt_args->save_file);
  int pl = bt_args->bt_info->piece_length;
  long file_size = bt_args->bt_info->length;
  long num_pieces = bt_args->bt_info->num_pieces;
  long piece_size = bt_args->bt_info->piece_length;
  long last_piece_size = file_size % piece_size;  

  // If last then it should be remaining file size
  int totalPieceLength = pl;
  int length = msg->length;
  int index = msg->payload.piece.index;  
  // Has already been download or already present - rogue present message
  if (piece_to_socket_map.find(index) == piece_to_socket_map.end())
    return;
  if (index == num_pieces - 1) 
    totalPieceLength = last_piece_size;
  char* piece = msg->payload.piece.piece;
  int begin = msg->payload.piece.begin;
  // Calcalate start 
  long start = (index * pl) + begin;
  
  assembleFile (fs,start, piece , length , false);
  long remaining_size = totalPieceLength - (begin + length);
  if (remaining_size <= 0) {
    // Verify the integrity of piece 
    // If SHA matches 
    // If SHA matches
    char id[20];
    char data[totalPieceLength];
    getPacketFile (fs,data,index * piece_size,totalPieceLength,false);
    calc_sha(data,totalPieceLength,id);
    char *hash = bt_args->bt_info->piece_hashes[index];
    bool flag = true;
    for (int m = 0; m < 20 ; m++) {
      if ((unsigned short)hash[m] != (unsigned short)id[m]) {
        flag = false;
        break;
      }
    }
    if (flag) {
      piece_to_socket_map.erase(index);
      // Send a have 
      completed_piece_to_socket_map.insert(std::make_pair(index,2));      
    } else {
      //or else send a request for the starting piece       
      int len;
      int blockSize = (1 << 15);
      blockSize = blockSize > totalPieceLength ? totalPieceLength : blockSize;
      std::cout << "Sending request from start now for I:L:B" << index <<" - "<< blockSize<<" - " << 0 << std::endl;
      char *msg = createRequestMessage(bt_args,len,index,blockSize , 0);
      int n = send(s,msg,len,0);
      if (n < 0) {
        std::cout << "R: Send Create request failed" << std::endl;
      }
    }
  } else {
    // Request next part of this piece     
    int len;
    int blockSize = (1 << 15);
    blockSize = blockSize > remaining_size ? remaining_size : blockSize;
    std::cout << "Sending request now for I:L:B" << index <<" - "<< blockSize<<" - " << begin+ length << std::endl;
    char *msg = createRequestMessage(bt_args,len,index,blockSize , begin + length);
    int n = send(s,msg,len,0);
    if (n < 0) {
      std::cout << "R: Send Create request failed" << std::endl;
    }
  }
}

int handleData(bt_args_t *bt_args, vector<char> vbuf , int s) {
  if (oldData.size() > 0)
    vbuf.insert(vbuf.begin(),oldData.begin(),oldData.end());
  int cbType = 0;
  char* start = &vbuf[0]; 
  char* buf = start;
  if (buf[0] == 19) {
    cbType = 0;
  } else {             
    cbType = 1;     
  }
  int done = -1;
  if (log_if (4.3))
    std::cout << "R: Vector Size  : "<< vbuf.size() << std::endl;  
  size_t totalSize = vbuf.size();
  while(1) {
    buf = start;
    // Just validate copy part of data required
    int sze = 0;
    if (buf[0] == 19)
      cbType = 0;
    else
      cbType = 1;
    switch(cbType) {      
    case 0 : {
      sze = sizeof(handshake_msg_t);      
    }break;
    case 1: {
      if (totalSize >= sizeof (bt_msg_t)) {
        bt_msg_t t;
        memcpy((char*)&t, buf, sizeof(bt_msg_t));
        std::cout << "Length from msg is "<< t.length << std::endl;
        sze = t.length + sizeof(t);
        done = 0;
      } else {        
        // Store data in temp var 
        done = 1;
      }
    }break;  
    default: {
       if (log_if(4.3))
         std::cout << "R: Buffer Overflow - SOme error  " << std::endl;
      // Do something with the data    
    }break;
    }
    if (done <= 0 && totalSize >= sze) {
      totalSize -= sze;
      start += sze;
      oldData.clear();      
    } else {
      // Store data in temp var 
      oldData.clear();
      for( int k = 0 ; k < totalSize ; k++){ 
        oldData.insert(oldData.end(),*start);
        start++;
      }
      std::cout << "Breaking " << done <<  " " << totalSize << " "<<sze<< std::endl;
      break;
    }
    
    /************* Now parse the data ***********/
    switch(cbType) {
    case 0 : {
      // Handshake message - Lets parse it 
      handshake_msg_t hmsg;
      hmsg.parse(buf);
      //bt_args->bitfieldMsg
      std::cout << "R: Peer Id " <<  hmsg.peerId << std::endl;    
      // int port = getSocketPort(s);
      // if (port != -1) {
      //   if (url_to_socket_map.find(string("localhost:") +std::to_string(port)) == url_to_socket_map.end()) {
      //     // send handshake
      //     url_to_socket_map.insert(std::make_pair(string("localhost:")+std::to_string(port), s));
      //   }
      // }
      int mc = sendBitFieldMessage(bt_args,s);      
      if(socket_to_piecelist_map.find(s) == socket_to_piecelist_map.end()) {
        sendHandshakeMsg(bt_args,s);
      }
    }break;
    case 1: {
      bt_msg_t msg;
      memset((char*)&msg,0x00,sizeof(msg));
      memcpy((char*)&msg , buf , sizeof(bt_msg_t));
      if (log_if(4.2)) {
        std::cout << "\nR: Data - Type : " << msg.bt_type << " Size : " << msg.length << std::endl; 
      }
      // Now use the type to parse message 
      switch(msg.bt_type) {
      case BT_HAVE: {
        int addedPiece = msg.payload.have;
        // Add to the map 
        registerPiece(s,addedPiece);
      }break;
      case BT_BITFILED: {
        bt_msg_t *m1 = parseBitField(buf, msg.length);
        if (log_if(4.3))
          std::cout << "R: Bitfield message "<< (unsigned short)(m1->payload.bitfiled.bitfield[0]) << std::endl;
        // Use the bitfield to assign a role to the Socket
        setPieceAvailability(bt_args,m1,s);
      }break; 
      case BT_REQUEST: {
        // msg already has reqyuired data - no need to reparse      
        if (log_if(4.3))
          std::cout << "R: Got a request - I:B:L " << msg.payload.request.index << " - " << msg.payload.request.begin << " - " << msg.payload.request.length  << std::endl;
        //sleep(1);
        sendData(bt_args,s,msg.payload.request.length,msg.payload.request.index,msg.payload.request.begin);
      }break;
      case BT_PIECE: {
        bt_msg_t *m1 = parsePieceMessage(buf, msg.length);
        if (log_if(4.3))
          std::cout << "R: Got data - I:B:L " << msg.payload.piece.index << " - " << msg.payload.piece.begin << " - " << msg.length<< std::endl;
        // Don't write if already written to
        if (piece_to_socket_map.find(msg.payload.piece.index) != piece_to_socket_map.end()) {
          writeToFile(bt_args,m1,s);
          //sleep(1);
        }
        // sleep(5);
        // if (log_if(2.4))
        //   std::cout << "R: Piece message "<< m1->payload.piece.piece << std::endl;
        
      }break;
      }
    }break;  
    default: {
      std::cout << "R: Buffer Overflow - SOme error  " << std::endl;
      // sleep(1);
      // Do something with the data    
    }break;
    }    
    if (done || totalSize <= 0)
      break;    
  }  
  return 0;
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
    if (port != ownPort && (url_to_socket_map.find("localhost:" + std::to_string(port)) == url_to_socket_map.end())) {
      sockaddr_in serv_addr;
      createSockAddr(port,&serv_addr);
      //create and connect as much as needed        
      if (create_and_connect(&serv_addr, efd , port) != 0) {
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
  while(1) {
    n = epoll_wait (efd,events,MAXEVENTS,1000);
    if (n > 0)  
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
          int dc =  sendHandshakeMsg(bt_args,events[i].data.fd);
          int mc = sendBitFieldMessage(bt_args,events[i].data.fd);
          if (dc < 0 || mc < 0) {
            std::cout << "SE: Handshake send failed" << std::endl;
            abort();
          } else {

            //handshake_socket_set.insert(events[i].data.fd);
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
    __npoll__(sockfd,handleData,sendRequestForPieces,bt_args);
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


 
