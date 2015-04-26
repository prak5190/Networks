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
  for (int i  = 0; i < num_pieces ; i++) {
    int k = (int) i/8;
    bzero(data,sizeof(data));
    long off  = piece_length < fileSize ? piece_length : fileSize;
    getPacketFile (name,data, i * piece_length,off , false);
    fileSize -= off;
    char id[20];
    calc_sha(data,off,id);  
    if (strncmp(id,pieces[i],20) == 0) {  
      if (i%8 == 0) {
        bitfield[k] = (unsigned char) 0;
      }
      // -2 Indicates piece exists     
      completed_piece_to_socket_map.insert(std::make_pair(i,-2));
      bitfield[k] = (unsigned char) ((1 << 7-(i%8))  + bitfield[k]);      
    } else {
      bitfield[k] = (unsigned char) ((1 << 7-(i%8))  + bitfield[k]);  
      // -1 indicates piece doesn't exist - Now populate this with sockets attempting to download it      
      if (piece_to_socket_map.find(i) == piece_to_socket_map.end()) {
        piece_to_socket_map.insert(std::make_pair(i,-1));
      }
    }
  }   
  closeFile(name);
  // The serialization of the message -
  msg.payload.bitfiled.size = sizeof (bitfield);
  msg.length = sizeof(bitfield);
  char *message= new char[msg.length];
  memcpy(message + sizeof(msg), bitfield , sizeof(bitfield));
  //msg.payload.bitfiled.bitfield = &message[sizeof(msg)];
  memcpy(message,(char*) &msg,sizeof(msg));  
  length1 = msg.length;
  // Set it in args
  bt_args->bitfieldMsg = message;
  bt_args->bitfield_length = length1;
  return message;
}

