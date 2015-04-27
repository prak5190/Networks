#include "common.cpp"

bt_msg_t* parseBitField(const char* message,int length) {
  bt_msg_t* msg1 = new bt_msg_t();
  memcpy((char*)msg1 , message , sizeof(bt_msg_t));  
  unsigned char* msg = new unsigned char[length];
  memcpy(msg,&message[sizeof(bt_msg_t)], length);
  msg1->payload.bitfiled.bitfield = msg;
  return msg1;
}

char* createRequestMessage(bt_args_t *bt_args,int &l1,int ind , int length , int begin) {
  // Return null message 
  if (length > (1 << 15) || ind > bt_args->bt_info->num_pieces || begin > bt_args->bt_info->piece_length) 
    return NULL;
  bt_msg_t msg;  
  msg.length = 0;
  msg.bt_type = BT_REQUEST;
  msg.payload.request.index = ind;
  msg.payload.request.length = length;     
  msg.payload.request.begin = begin;
  char *message = new char[sizeof(bt_msg_t)];
  l1 =  sizeof(bt_msg_t);  
  memcpy(message,(char*)&msg,sizeof(bt_msg_t));
  return message;   
};

bt_msg_t* parsePieceMessage(const char *message, int length) {
  bt_msg_t *msg1 = new bt_msg_t();
  memcpy((char*)msg1 , message , sizeof(bt_msg_t));  
  char* msg = new char[length];
  memcpy(msg,&message[sizeof(bt_msg_t)], length);
  msg1->payload.piece.piece = msg;
  return msg1;
};
char* createPieceMessage(bt_args_t *bt_args, int &l1, char *data , int size , int index,int begin) {
  bt_msg_t msg;  
  msg.length = size;
  msg.bt_type = BT_PIECE;
  msg.payload.piece.index = index;
  l1 =  sizeof(bt_msg_t) + size;
  char *message = new char[l1];
  memcpy(message,(char*)&msg,sizeof(bt_msg_t));
  memcpy(message + sizeof(msg),data,size);
  return message;  
}

int* bitToIntArray(char *bits , int length) {
  int *arr = new int[length];
  int size = ceil((double)length/8);
  int totalPieces = length;
  int p = 0;
  for (int i = 0; i < size ; i++) {
    unsigned char a = bits[i];
    // Check every bit
    for (int j = 0; j < 8 ; j++) {
      if (p < totalPieces) {
        if (((a >> (7 - j)) & 1) == 1) {
          arr[p] = 1;
        } else {
          arr[p] = 0;
        }
      } else {
        break;
      }
      p++;
    }
    if (p >= totalPieces) 
      break;
  }
  return arr;
}

char* createBitfieldMessage(bt_args_t *bt_args,int &length1) {
  bt_msg_t msg;  
  msg.bt_type = BT_BITFILED;
  int num_pieces = bt_args->bt_info->num_pieces;
  long length = bt_args->bt_info->length;
  char** pieces = bt_args->bt_info->piece_hashes;
  int piece_length = bt_args->bt_info->piece_length;
  char data[piece_length];
  char* fname = bt_args->save_file;
  string name = string(fname);
  long fileSize = getFileSize(name);
  if (length < fileSize) {
    // Do something to shrink the file
  }
  unsigned char *bitfield = new unsigned char[(int)ceil((double)num_pieces/8)]; 
  int k = 0;
  for (int i  = 0; i < num_pieces ; i++) {    
    k = (int) i/8;
    if (i%8 == 0) {
      bitfield[k] = (unsigned char) 0;
    }    
    bzero(data,sizeof(data));
    long off  = piece_length < fileSize ? piece_length : fileSize;
    getPacketFile (name,data, i * piece_length,off , false);
    fileSize -= off;
    char id[20];
    calc_sha(data,off,id);            
    bool flag = true;
    for (int m = 0; m < 20 ; m++) {
      if ((unsigned short)pieces[i][m] != (unsigned short)id[m]) {
        flag = false;
        break;
      }
    }
    // if (strncmp(id,pieces[i],20) == 0) {  
    if (flag) {
      // std::cout << "Bitfield Matched for piecce " << i << std::endl;      
      bitfield[k] = (unsigned char) ((1 << (7-(i%8)))  + bitfield[k]);      
    } else {
      //bitfield[k] = (unsigned char) ((1 << 7-(i%8))  + bitfield[k]);  
      // -1 indicates piece doesn't exist - Now populate this with sockets attempting to download it      
      // std::cout << "Bitfield Failed for piecce " << i << std::endl;
      if (piece_to_socket_map.find(i) == piece_to_socket_map.end()) {
        piece_to_socket_map.insert(std::make_pair(i,-1));
      }
    }
  }
  if (log_if(4.4)) {
    std::cout << "Init: BitString ";
    for(int i = 0; i < num_pieces ; i++) {
      std::cout << ( ((unsigned char)bitfield[k]) >> (7 - (i%8)));
    }
    std::cout <<  std::endl;
  }

  closeFile(name);
  // The serialization of the message -
  msg.payload.bitfiled.size = sizeof (bitfield);
  msg.length = sizeof(bitfield);
  char *message= new char[sizeof(msg) + msg.length];
  memcpy(message + sizeof(msg), bitfield , sizeof(bitfield));  
  memcpy(message,(char*) &msg,sizeof(msg));  
  length1 = msg.length;
  // Set it in args
  bt_args->bitfieldMsg = message;
  bt_args->bitfield_length = length1;
  return message;
}


int sendData(bt_args_t *bt_args, int s, int length, int index, int begin) {
  // Calculate start of file 
  long pl = bt_args->bt_info->piece_length;
  long start = index * pl + begin; 
  string sf = string(bt_args->save_file);
  char data[length];
  if (index == bt_args->bt_info->num_pieces) {
    // DO something to demarcate end of file
  }
  getPacketFile(sf,data, start,length, false);
  char message[sizeof(bt_msg_t) + length];
  bt_msg_t t; 
  t.bt_type = 7;
  t.length = length;
  t.payload.piece.index = index;
  t.payload.piece.begin = begin; 
  memcpy(message,(char*)&t,sizeof(t));
  memcpy(&message[sizeof(t)],data,length);
  int n = send(s,message,sizeof(message),0);
  if (n >= 0) {
    std::cout << "S: Data Sent of length " << length  << std::endl;
  } else {
    std::cout << "SE: Error sending piece " << std::endl;
    return -1;
  }
  return 0;
}
int sendRequestForPieces(bt_args_t *bt_args,int s) {
  auto it1 = socket_to_piecelist_map.find(s);
  int requ = 0;
  if (piece_to_socket_map.size() == 0) {
    std::cout << "S: Am a seeder - No need to send a request " << std::endl;
    return 1;
  }
  long file_size = bt_args->bt_info->length;
  long num_pieces = bt_args->bt_info->num_pieces;
  long piece_size = bt_args->bt_info->piece_length;
  long last_piece_size = file_size % piece_size;  
  std::cout << "S: Pieces available in socket " << s << " : " << it1->second.size() << std::endl;
  for (auto it = piece_to_socket_map.begin(); it != piece_to_socket_map.end(); ++it) {
    std::cout << "Required piece " << it->first << std::endl;;
  }
  for (auto it = it1->second.begin(); it != it1->second.end(); ++it) {
    int length = 0;
    int piece = *it;
    std::cout << "S: Piece Available is " << piece << std::endl;
    // If piece is not in file - it will exist in map
    auto it2 = piece_to_socket_map.find(piece);
    if (it2 != piece_to_socket_map.end()){      
      // If positive - then it is already being downloaded by the socket 
      if (it2->second < 0){
        requ++;
        // 
        int blockSize = (1 << 15);
        // This is the last piece
        if (piece == num_pieces - 1) 
          blockSize = last_piece_size;
        // Start request 
        char *msg = createRequestMessage(bt_args,length,piece,blockSize , 0);
        int n = send(s,msg,length,0);
        piece_to_socket_map.insert(std::make_pair(piece,s));
        if (n > 0){
          std::cout << "S: Sending request *********************"<< s << "    " << std::endl;       
        } else {
          std::cout << "SE: Failed " << strerror(errno) << std::endl;
        }
      }
    }
  }
  if (requ == 0) 
    std::cout << "S: No Useful data found from socket "<< s << std::endl;
  return 0;
}

int sendHandshakeMsg(bt_args_t *bt_args, int s) {
  handshake_msg_t msg , k;
  msg.setData(bt_args->bt_info->info_hash,string("dasda11111s"));     
  char message[sizeof(msg)];  
  memcpy(message , (char*) &msg , sizeof(handshake_msg_t));
  std::cout << "HAND: Sending Message "<<sizeof(handshake_msg_t)  << std::endl;
  k.parse(message);
  std::cout << "HAND: Parse ID " << k.peerId << std::endl;
  int n = send(s,message,sizeof(handshake_msg_t),0);
  if (n >= 0) {
    registerSocket(s);
  }
  return n;
}


int sendBitFieldMessage(bt_args_t *bt_args , int s) {  
  char* m = bt_args->bitfieldMsg;
  int length = bt_args->bitfield_length;
  int n = send(s,m,sizeof(bt_msg_t) + length,0); 
  //bt_msg_t *mgs = parseBitField(m,length);
  if (n < 0) {
    std::cout << "S:Bitfield Message sending failed  "<< s << std::endl;
    return -1;
  } 
  return 0;  
}
