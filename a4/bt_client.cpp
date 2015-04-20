#include "common.cpp"  
#include "bt_lib.h"
#include "udphandler.cpp"
#include "bt_setup.h"
 
void parseCmdArgs(bt_args_t &bt_args, int argc,  char * argv[]) {
  parse_args(&bt_args, argc, argv);
  if (bt_args.verbose != 0)     
    LOG_LEVEL = bt_args.verbose;
  if(log_if(4.1)) {   
    printf("Args:\n");   
    printf("verbose: %f\n",bt_args.verbose);
    printf("save_file: %s\n",bt_args.save_file);
    printf("log_file: %s\n",bt_args.log_file); 
    printf("torrent_file: %s\n", bt_args.torrent_file);
    int num = 0;
    for(int i=0;i<MAX_CONNECTIONS;i++){
      if(bt_args.peers[i] != NULL) {
        print_peer(bt_args.peers[i]);
        num++;
      }      
    }      
    // Hard code for testing
    if (num == 0) 
      bt_args.port = 9000; 
    bt_args.num_peers = num;
    if (log_if(4.1))
      std::cout << "Number of peers " << num << std::endl;
  }
};

void parseTorrentFile(bt_args_t &bt_args) {
  //read and parse the torrent file here  
  string name = string(bt_args.torrent_file);
  long size = getFileSize(name);  
  if (size >= 0) {
    if (log_if(4))
      std::cout << "File Size " << size  << std::endl;
    char data[size+1];    
    getPacketFile (name, data, 0,  size, true);
    data[size] = '\0';
    int k = 1;
    std::unordered_map<string,BItem> fmap;
    parseDict(fmap,data,k,"");
    logTorrentInfo(fmap);
    bt_info_t infoDet;
    populateInfo(fmap , &infoDet); 
    bt_args.bt_info = &infoDet;    
  } else {
    // Error case
    std::cout << "File doesn't exit " << name << std::endl;
  }
}
  
bt_args_t bt_args;
int main (int argc, char * argv[]){
  parseCmdArgs(bt_args,argc,argv);   
  parseTorrentFile(bt_args);
  int s = get_and_bind_socket(&bt_args);
  pthread_t sth ,rth; 
  if (s != -1) {
    rth = startRecieverThread(&bt_args,s);            
    if (bt_args.num_peers > 0) { 
      sth = startSenderThread(&bt_args,s); 
    }
  }
  // while(1){
  //   std::cout << "Working "<<i  << std::endl;
  //   i++; 
  //   //try to accept incoming connection from new peer
  //   break;
       
  //   //poll current peers for incoming traffic
  //   //   write pieces to files
  //   //   udpdate peers choke or unchoke status      
  //   //   responses to have/havenots/interested etc.
  //   //for peers that are not choked
  //   //   request pieaces from outcoming traffic

  //   //check livelenss of peers and replace dead (or useless) peers      
  //   //with new potentially useful peers
    
  //   //update peers, 
 
  // }
   
  if (rth != 0) {
    std::cout << "joining reci thread " << std::endl;
    pthread_join(rth,NULL);  
  }
  if (sth != 0) {
    std::cout << "joining send  thread " << std::endl;
    pthread_join(sth,NULL);
  } 
  return 0;
}   
 
 
