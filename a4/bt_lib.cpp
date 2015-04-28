#ifndef __BT_LIB__
#define __BT_LIB__ 1 

#include "common.cpp"

/*Maximum file name size, to make things easy*/
#define FILE_NAME_MAX 1024

/*Maxium number of connections*/
#define MAX_CONNECTIONS 5

/*initial port to try and open a listen socket on*/
#define INIT_PORT 6667 

/*max port to try and open a listen socket on*/
#define MAX_PORT 6699

/*Different BitTorrent Message Types*/ 
#define BT_CHOKE 0
#define BT_UNCHOKE 1
#define BT_INTERSTED 2
#define BT_NOT_INTERESTED 3
#define BT_HAVE 4
#define BT_BITFILED 5
#define BT_REQUEST 6
#define BT_PIECE 7
#define BT_CANCEL 8

/*size (in bytes) of id field for peers*/
#define ID_SIZE 20


//holds information about a peer
typedef struct peer{
  unsigned char id[ID_SIZE]; //the peer id
  unsigned short port; //the port to connect n
  struct sockaddr_in sockaddr; //sockaddr for peer
  int choked; //peer choked?
  int interested; //peer interested?
}peer_t;

typedef struct handshake_msg{
  char init[20];
  char res[8];
  char hash[20];
  char peerId[20];
  void setData(char *info_hash, string peerId) {   
    memcpy(this->hash,info_hash,20);
    strncpy(this->peerId , peerId.c_str() , sizeof(this->peerId));
    this->init[0] = (char)19;
    strcpy(this->init + 1, "BitTorrent protocol");
    bzero(this->res,sizeof(this->res)); 
  };
  
  void parse(const char* msg) {    
    memcpy(this->init , msg , sizeof(this->init)); msg += sizeof(this->init);
    memcpy(this->res , msg , sizeof(this->res)); msg += sizeof(this->res);
    memcpy(this->hash , msg , sizeof(this->hash)); msg += sizeof(this->hash);
    memcpy(this->peerId , msg , sizeof(this->peerId)); msg += sizeof(this->peerId);
  }
}handshake_msg_t;



//holds information about a torrent file
typedef struct {
  char name[FILE_NAME_MAX]; //name of file
  int piece_length; //number of bytes in each piece
  int length; //length of the file in bytes
  int num_pieces; //number of pieces, computed based on above two values
  char info_hash[20];
  char ** piece_hashes; //pointer to 20 byte data buffers containing the sha1sum of each of the pieces
} bt_info_t;


//holds all the agurments and state for a running the bt client
typedef struct {
  float verbose; //verbose level
  char save_file[FILE_NAME_MAX];//the filename to save to
  FILE * f_save;
  char log_file[FILE_NAME_MAX];//the log file
  char torrent_file[FILE_NAME_MAX];// *.torrent file
  peer_t * peers[MAX_CONNECTIONS]; // array of peer_t pointers
  unsigned int id; //this bt_clients id
  int sockets[MAX_CONNECTIONS]; //Array of possible sockets
  int num_peers;
  char *bitfieldMsg;
  int bitfield_length;
  //struct pollfd poll_sockets[MAX_CONNECTIONS]; //Array of pollfd for polling for input  
  /* set once torrent is parsed */
  bt_info_t * bt_info; //the parsed info for this torrent 
  int port;
} bt_args_t;


/**
 * Message structures
 **/

typedef struct {
  unsigned char * bitfield; //bitfield where each bit represents a piece that the peer has or doesn't have
  size_t size;//size of the bitfiled
} bt_bitfield_t;

typedef struct{
  int index; //which piece index
  int begin; //offset within piece
  int length; //amount wanted, within a power of two
} bt_request_t;

typedef struct{
  int index; //which piece index
  int begin; //offset within piece
  char *piece; //pointer to start of the data for a piece
} bt_piece_t;



typedef struct bt_msg{
  int length; //length of remaining message, 
              //0 length message is a keep-alive message
  unsigned int bt_type;//type of bt_mesage

  //payload can be any of these
  union { 
    bt_bitfield_t bitfiled;//send a bitfield
    int have; //what piece you have
    bt_piece_t piece; //a peice message
    bt_request_t request; //request messge
    bt_request_t cancel; //cancel message, same type as request
    char data[0];//pointer to start of payload, just incase
  }payload;  

} bt_msg_t;



void calc_sha(char * data,int len,char *id){
  //id is just the SHA1 of the ip and port string
  SHA1((unsigned char *) data, len, (unsigned char *) id); 
  return;
}

void calc_id(char * ip, unsigned short port, char *id){
  char data[256];
  int len;
  
  //format print
  len = snprintf(data,256,"%s%u",ip,port);

  //id is just the SHA1 of the ip and port string
  SHA1((unsigned char *) data, len, (unsigned char *) id); 
  return;
}


/**
 * init_peer(peer_t * peer, int id, char * ip, unsigned short port) -> int
 *
 *
 * initialize the peer_t structure peer with an id, ip address, and a
 * port. Further, it will set up the sockaddr such that a socket
 * connection can be more easily established.
 *
 * Return: 0 on success, negative values on failure. Will exit on bad
 * ip address.
 *   
 **/  
int init_peer(peer_t *peer, char * id, char * ip, unsigned short port){
    
  struct hostent * hostinfo;
  //set the host id and port for referece
  memcpy(peer->id, id, ID_SIZE);
  peer->port = port;
    
  //get the host by name
  if((hostinfo = gethostbyname(ip)) ==  NULL){
    perror("gethostbyname failure, no such host?");
    herror("gethostbyname");
    exit(1);
  }
  
  //zero out the sock address
  bzero(&(peer->sockaddr), sizeof(peer->sockaddr));
      
  //set the family to AF_INET, i.e., Iternet Addressing
  peer->sockaddr.sin_family = AF_INET;
    
  //copy the address to the right place
  bcopy((char *) (hostinfo->h_addr), 
        (char *) &(peer->sockaddr.sin_addr.s_addr),
        hostinfo->h_length);
    
  //encode the port
  peer->sockaddr.sin_port = htons(port);
  
  return 0;

}

/**
 * print_peer(peer_t *peer) -> void
 *
 * print out debug info of a peer
 *
 **/
void print_peer(peer_t *peer){
  int i;

  if(peer){
    printf("peer: %s:%u ",
           inet_ntoa(peer->sockaddr.sin_addr),
           peer->port);
    printf("id: ");
    for(i=0;i<ID_SIZE;i++){
      printf("%02x",peer->id[i]);
    }
    printf("\n");
  }
}



/**
 * usage(FILE * file) -> void
 *
 * print the usage of this program to the file stream file
 *
 **/

void usage(FILE * file){
  if(file == NULL){
    file = stdout;
  }

  fprintf(file,
          "bt-client [OPTIONS] file.torrent\n"
          "  -h            \t Print this help screen\n"
          "  -b ip         \t Bind to this ip for incoming connections, ports\n"
          "                \t are selected automatically\n"
          "  -s save_file  \t Save the torrent in directory save_dir (dflt: .)\n"
          "  -l log_file   \t Save logs to log_filw (dflt: bt-client.log)\n"
          "  -p ip:port    \t Instead of contacing the tracker for a peer list,\n"
          "                \t use this peer instead, ip:port (ip or hostname)\n"
          "                \t (include multiple -p for more than 1 peer)\n"
          "  -I id         \t Set the node identifier to id (dflt: random)\n"
          "  -v            \t verbose, print additional verbose info\n");
} 

/**
 * __parse_peer(peer_t * peer, char peer_st) -> void
 *
 * parse a peer string, peer_st and store the parsed result in peer
 *
 * ERRORS: Will exit on various errors
 **/

void __parse_peer(peer_t * peer, char * peer_st){
  char * parse_str;
  char * word;
  unsigned short port;
  char * ip;
  char id[20];
  char sep[] = ":";
  int i;

  //need to copy becaus strtok mangels things
  parse_str = (char*)malloc(strlen(peer_st)+1);
  strncpy(parse_str, peer_st, strlen(peer_st)+1);

  //only can have 2 tokens max, but may have less
  for(word = strtok(parse_str, sep), i=0; 
      (word && i < 3); 
      word = strtok(NULL,sep), i++){

    printf("%d:%s\n",i,word);
    switch(i){
    case 0://id
      ip = word;
      break;
    case 1://ip
      port = atoi(word);
    default:
      break;
    }

  }

  if(i < 2){
    fprintf(stderr,"ERROR: Parsing Peer: Not enough values in '%s'\n",peer_st);
    usage(stderr);
    exit(1);
  }

  if(word){
    fprintf(stderr, "ERROR: Parsing Peer: Too many values in '%s'\n",peer_st);
    usage(stderr);
    exit(1);
  }


  //calculate the id, value placed in id
  calc_id(ip,port,id);

  //build the object we need
  init_peer(peer, id, ip, port);
  
  //free extra memory
  free(parse_str);

  return;
}

/**
 * pars_args(bt_args_t * bt_args, int argc, char * argv[]) -> void
 *
 * parse the command line arguments to bt_client using getopt and
 * store the result in bt_args.
 *
 * ERRORS: Will exit on various errors
 *
 **/ 
void parse_args(bt_args_t * bt_args, int argc,  char * argv[]){
  int ch; //ch for each flag
  int n_peers = 0;
  int i;

  /* set the default args */
  bt_args->verbose=0; //no verbosity
  
  //null save_file, log_file and torrent_file
  memset(bt_args->save_file,0x00,FILE_NAME_MAX);
  memset(bt_args->torrent_file,0x00,FILE_NAME_MAX);
  memset(bt_args->log_file,0x00,FILE_NAME_MAX);
  
  //null out file pointers
  bt_args->f_save = NULL;

  //null bt_info pointer, should be set once torrent file is read
  bt_args->bt_info = NULL;

  //default lag file
  strncpy(bt_args->log_file,"bt-client.log",FILE_NAME_MAX);
  
  for(i=0;i<MAX_CONNECTIONS;i++){
    bt_args->peers[i] = NULL; //initially NULL
  }

  bt_args->id = 0;
  
  while ((ch = getopt(argc, argv, "hp:s:l:v:I:")) != -1) {
    switch (ch) {
    case 'h': //help
      usage(stdout);
      exit(0);
      break;
    case 'v': //verbose
      bt_args->verbose =  atof(optarg);
      break;
    case 's': //save file
      strncpy(bt_args->save_file,optarg,FILE_NAME_MAX);
      break;
    case 'l': //log file
      strncpy(bt_args->log_file,optarg,FILE_NAME_MAX);
      break;
    case 'p': //peer
      n_peers++;
      //check if we are going to overflow
      if(n_peers > MAX_CONNECTIONS){
        fprintf(stderr,"ERROR: Can only support %d initial peers",MAX_CONNECTIONS);
        usage(stderr);
        exit(1);
      }

      bt_args->peers[n_peers] = (peer_t*)malloc(sizeof(peer_t));

      //parse peers
      __parse_peer(bt_args->peers[n_peers], optarg);
      break;
    case 'I':
      bt_args->id = atoi(optarg);
      break;
    default:
      fprintf(stderr,"ERROR: Unknown option '-%c'\n",ch);
      usage(stdout);
      exit(1);
    }
  }


  argc -= optind;
  argv += optind;

  if(argc == 0){
    fprintf(stderr,"ERROR: Require torrent file\n");
    usage(stderr);
    exit(1);
  }

  //copy torrent file over
  strncpy(bt_args->torrent_file,argv[0],FILE_NAME_MAX);

  return ;
}

struct qMem {
  int s;
  string data;
};

// Useless
std::unordered_map<string,int> url_to_socket_map;
std::set<int> handshake_socket_set;
std::unordered_map<int,int> completed_piece_to_socket_map;
std::unordered_map<int,int> piece_to_socket_map;
std::unordered_map<int,int> piece_to_lastN_map;
std::unordered_map<int,int> socket_to_lastN_map;

// Both of these need to be in sync
// Stores the socket to pieces it can download
std::unordered_map<int,std::set<int>> socket_to_piecelist_map;

// Stores the piece to sockets it can download
std::unordered_map<int,std::set<int>> piece_to_socketlist_map;


double lastProgress=0 ;
int LastPrint = 0;
int LastType = -1;
int LastLength = -1;
int LastSocket = -1 ;
void printProgress(bt_args_t *args) {
  // long size = args->bt_info->length;
  long num_pieces = args->bt_info->num_pieces;
  int pieces_left = piece_to_socket_map.size();
  int num_sockets = socket_to_piecelist_map.size();
  double progress = ((double)(num_pieces - pieces_left)/(double) num_pieces) * (double)100;  
  string log;
  string peerMessageInfo = "";
  if (LastType != -1 && LastLength != -1 && LastSocket != -1) 
    peerMessageInfo = string(" Last Message details - T : ") + std::to_string(LastType) + " L: " + std::to_string(LastLength) + " Sock: "+std::to_string(LastSocket);
  // print log to error console
  log = "[" + getTimeStamp("%F %T") + "] ";
  if (progress < 100.0) {
    std::cout << "\33[2K\r"<< log <<"Progress : "<< progress << "% | " << "Connections : " << num_sockets << peerMessageInfo;
  } else if (lastProgress < 100.0 && progress >= 100.0) {
    std::cout << "\33[2K\r"<< log <<"Progress : "<< progress << "% | " << "Download completed " << peerMessageInfo;
  } else {
    if (LastPrint % 10 == 0) 
      std::cout << "\33[2K\r"<< log <<"Seeding  "<< peerMessageInfo;
    LastPrint++;
    if (LastPrint >= 101) 
      LastPrint = LastPrint % 100;
  }
  
  lastProgress = progress;
  std::cout.flush();  
}
int registerPiece(int s,int pieceIndex) {
  auto it = socket_to_piecelist_map.find(s);
  if (it == socket_to_piecelist_map.end()) {
    // Create a set and insert it 
    std::set<int> tmp;
    tmp.insert(pieceIndex);
    socket_to_piecelist_map.insert(std::make_pair(s,tmp));
    if(log_if(4.3))
      std::cout << "R:App: Added piece to socket to piece list, Size : "<< tmp.size() << std::endl;  
  } else { 
    it->second.insert(pieceIndex);
    if(log_if(4.3))
      std::cout << "R:App: Added piece to socket to piece list, Size : "<< it->second.size() << std::endl;  
  }  
  // Insert rmap
  it = piece_to_socketlist_map.find(pieceIndex);
  if (it == piece_to_socketlist_map.end()) {
    std::set<int> tmp;
    tmp.insert(s);
    socket_to_piecelist_map.insert(std::make_pair(s,tmp));
  } else {
    it->second.insert(s);
  }  
  return 0;
}
int unregisterSocket(int s) {
  // Delete the socket entry 
  socket_to_lastN_map.erase(s);
  socket_to_piecelist_map.erase(s);
  // Not removing dummy sockets from pieces, Instead delete it when it is accessed  
  return 0;
}
int registerSocket(int s) {
  auto it = socket_to_piecelist_map.find(s);
  if (it == socket_to_piecelist_map.end()) {
    std::set<int> tmp;    
    string logM;
    socket_to_piecelist_map.insert(std::make_pair(s,tmp));
    logM = "[" + getTimeStamp("%F %T") + "] ";  
    std::cout << "\n\33[2K\r"<< logM <<"Handshake done for Socket " << s;
  }
  return 0;
}

bool isSocketAlive(int s) {
  return socket_to_piecelist_map.find(s) == socket_to_piecelist_map.end() ? false : true;
}

int TIMER = 0;
void renewPiece(int p) {
  int n = TIMER;
  auto it = piece_to_lastN_map.find(p);
  if(it == piece_to_lastN_map.end())
    piece_to_lastN_map.insert(std::make_pair(p,n));
  else
    it->second = n;
}
void renewSocket(int s) {
  int n = TIMER;
  auto it = socket_to_lastN_map.find(s);
  if(it == socket_to_lastN_map.end())
    socket_to_lastN_map.insert(std::make_pair(s,n));
  else
    it->second = n;
}
void checkAndKillPiece() {
  int n = TIMER;
  for (auto it = piece_to_lastN_map.begin(); it != piece_to_lastN_map.end(); ++it) {
    int lastN = it->second; 
    it->second = n;
    int p = it->first;
    if (piece_to_socket_map.find(p) == piece_to_socket_map.end()) {
      // Already downloaded - just remove it 
      piece_to_lastN_map.erase(p);
      continue;
    }
    // More than 10 wait loops
    if (n - lastN  > 10) {
      // The socket is blacked out
      int s = piece_to_socket_map.find(p)->second;
      piece_to_socket_map.find(p)->second = -1;
      // Kill socket too
      if (socket_to_lastN_map.find(s) != socket_to_lastN_map.end()) {
        if (n - socket_to_lastN_map.find(s)->second > 10) {
          unregisterSocket(s);
        }
      } else {
        unregisterSocket(s);
      }
    }
  }
}

std::vector<char> oldData;
#endif

