#include "common.cpp"  
#include "netHandler.cpp"
#include "bt_lib.cpp" 
 
void parseCmdArgs(bt_args_t *bt_args, int argc,  char * argv[]) {
  parse_args(bt_args, argc, argv);
  if (bt_args->verbose != 0)     
    LOG_LEVEL = bt_args->verbose;
  printf("save_file: %s\n",bt_args->save_file);
  if(log_if(4.1)) {   
    printf("Args:\n");   
    printf("verbose: %f\n",bt_args->verbose);
    printf("save_file: %s\n",bt_args->save_file);
    printf("log_file: %s\n",bt_args->log_file); 
    printf("torrent_file: %s\n", bt_args->torrent_file);
  }
  int num = 0;
  for(int i=0;i<MAX_CONNECTIONS;i++){
    if(bt_args->peers[i] != NULL) {
      if(log_if(4.1))
        print_peer(bt_args->peers[i]);
      num++;
    }      
  }    
  bt_args->num_peers = num;
  if (log_if(4.1))
    std::cout << "Number of peers " << num << std::endl;
  
};

void parseTorrentFile(bt_args_t *bt_args) {
  //read and parse the torrent file here  
  string name = string(bt_args->torrent_file);
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
    bt_info_t *infoDet = new bt_info_t();
    populateInfo(fmap , infoDet); 
    bt_args->bt_info = infoDet;    
  } else {
    // Error case
    std::cout << "File doesn't exit " << name << std::endl;
  }
}
  
bt_args_t *bt_args = new bt_args_t();
int main (int argc, char * argv[]){  
  parseCmdArgs(bt_args,argc,argv);   
  parseTorrentFile(bt_args);
  // Ensure save file is of correct size
  string fname = string(bt_args->save_file);
  long size = getFileSize(fname);
  if (size < bt_args->bt_info->length)
    createFile(fname, bt_args->bt_info->length);

  int length;
  // Populate bit field and maps
  createBitfieldMessage(bt_args,length);
  int s = get_and_bind_socket2(bt_args);  
  int err = make_socket_non_blocking(s);
  if (err == -1) {
    std::cout << "Not able to make socket unblocking " << std::endl;
    abort();
  }
  err = listen(s,SOMAXCONN);
  if (err ==-1) {
    std::cout << "Unable to listen " << std::endl;    
    abort();
  }

  pthread_t sth ,rth , hth; 
  if (s != -1) {
    rth = startRecieverThread(bt_args,s);
    sth = startSenderThread(bt_args,s);
    //hth = startHandlerThread(bt_args,s);
  }
  
  if (rth != 0) {
    std::cout << "joining reci thread " << std::endl;
    pthread_join(rth,NULL);  
  }
  // if (hth != 0) {
  //   std::cout << "joining handler thread " << std::endl;
  //   pthread_join(hth,NULL);  
  // }
  if (sth != 0) {
    std::cout << "joining send  thread " << std::endl;
    pthread_join(sth,NULL);
  } 
  return 0;
}   
 
 
