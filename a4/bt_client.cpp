#include "common.cpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h> //ip hdeader library (must come before ip_icmp.h)
#include <netinet/ip_icmp.h> //icmp header
#include <arpa/inet.h> //internet address library
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include "bencode.cpp"
#include "bt_lib.h"
#include "bt_setup.h"

void __parse_torrent(bt_info_t* peer, char * peer_st){
  /*
  char name[FILE_NAME_MAX]; //name of file
  int piece_length; //number of bytes in each piece
  int length; //length of the file in bytes
  int num_pieces; //number of pieces, computed based on above two values
  char ** piece_hashes; //pointer to 20 byte data buffers containing the sha1sum of each of the pieces
  */
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
  //init_peer(peer, id, ip, port);
  
  //free extra memory
  free(parse_str);

  return;
}

int main (int argc, char * argv[]){

  bt_args_t bt_args;
  int i;

  parse_args(&bt_args, argc, argv);

  if(bt_args.verbose){
    printf("Args:\n");
    printf("verbose: %d\n",bt_args.verbose);
    printf("save_file: %s\n",bt_args.save_file);
    printf("log_file: %s\n",bt_args.log_file);
    printf("torrent_file: %s\n", bt_args.torrent_file);

    for(i=0;i<MAX_CONNECTIONS;i++){
      if(bt_args.peers[i] != NULL)
        print_peer(bt_args.peers[i]);
    }

    
  }

  //read and parse the torrent file here  
  string name = string(bt_args.torrent_file);
  long size = getFileSize(name);  
  if (size >= 0) {
    if (log(4))
      std::cout << "File Size " << size  << std::endl;
    char data[size+1];    
    getPacketFile (name, data, 0,  size, true);
    data[size] = '\0';
    int k = 1;
    std::unordered_map<string,BItem> fmap;
    parseDict(fmap,data,k,"");
    std::cout << fmap.size() << std::endl;
    logTorrentInfo(fmap);
  } else {
    // Error case
    std::cout << "File doesn't exit " << name << std::endl;
  }
  //peer_t peerData;
  //__parse_peer(&peerData,data);
  /* if(bt_args.verbose){ */
  /*   printf("Data %s\n",data); */
  /*   printf("Peer Data %s \n",peerData.id); */
  /*   // print out the torrent file arguments here */
    
  /* } */
  
  //main client loop
  printf("Starting Main Loop\n");
  while(1){
    break;
    //try to accept incoming connection from new peer
       
    
    //poll current peers for incoming traffic
    //   write pieces to files
    //   udpdate peers choke or unchoke status
    //   responses to have/havenots/interested etc.
    //for peers that are not choked
    //   request pieaces from outcoming traffic

    //check livelenss of peers and replace dead (or useless) peers
    //with new potentially useful peers
    
    //update peers, 

  }

  return 0;
}
