#include "common.cpp"
#ifndef __BENCODE__
#define __BENCODE__ 1 
long parseNum(char *data, int &n) {
  char a[20];
  int i=0;
  while(data[n] != 'e') {
    a[i] = data[n];
    i++;  
    n++;
  }
  a[i] = '\0';
  n++;
  if (log_if(3))
    std::cout << "Num String "<< a << std::endl;
  return atol(a);
};

long parseSize(char* data,int &n) {
  char a[20];
  int i=0;
  while(data[n] != ':' &&  data[n] != '\0') {    
    a[i] = data[n];
    i++;
    n++;
  }
  a[i] = '\0';
  n++;
  return atol(a);  
};
void copyString(char*data,int &n ,char* str,int size) {
  data = &data[n];
  int i;
  for(i=0; i < size;i++) {
    str[i] = *data;
    data++;
  }
  n += i;
}

struct BItem
{
  long* val;
  long numVal;
  int type; // 0 - Str , 1 - Num , 2 - dict , 3 -list
  size_t size;
};


string parseDict(std::unordered_map<std::string,BItem> &map,char* data, int &n,string prefix) {
  long size = 0;
  bool isKey = true;
  char* strKey,*str;  
  int type = 0; // 0 - Str , 1 - Num , 2 - dict , 3 -list
  if (log_if(4.1))
    std::cout << "Torrent String "<<data << std::endl;
  int start = n ;
  while(data[n]!='\0') {    
    // Represents string 
    long num;
    if(log_if(3.2))
      std::cout << "MEta "<< data[n] << std::endl;
    if (data[n] >= 48 && data[n] < 58) {
      size = parseSize(data,n);
      if (log_if(3))
        std::cout << "Size is " << size << " : "<< n << std::endl;
      str = new char[size+1];
      bzero(str,size+1);
      str[size] = '\0';
      copyString(data,n,str,size);
      if (log_if(3))
        std::cout << "String : "<< str << std::endl;
      if (isKey) {
        strKey = str;
      }
      type = 0;
    } else if (data[n] == 'i') {
      n++;
      num = parseNum(data,n);      
      if (log_if(3))
        std::cout << "Number " << num << std::endl;
      type = 1;
    } else if (data[n] == 'd') {        
      n++;
      string newPrefix = prefix + string(strKey) ;
      string dictStr = parseDict(map,data,n, newPrefix + ".");      
      if (newPrefix.length() != 0) {
        BItem dictItem;
        dictItem.size= dictStr.length();
        dictItem.type = 0;
        dictItem.val = (long*)(dictStr.c_str());
        std::cout << "Inserting prefix " << prefix << std::endl;
        map.insert(std::make_pair(newPrefix,dictItem));
      }
      type = 2;n++;
    } else if (data[n] == 'l') {
      type = 3;n++;
    } else if (data[n] == 'e') {
      break;
    } else {
      break;
    }
    
    if (!isKey) {
      // Print out stuff    
      if (log_if(3.5))
        std::cout << "Key : "<< strKey <<" - ";
      string key = prefix +  string(strKey);
      BItem temp;
      temp.size = size;
      temp.type = type;
      switch (type) {
      case 0 : 
        if(log_if(3.5))
          std::cout << str << std::endl;
        temp.val = (long*)str;
        map.insert(std::make_pair(key,temp));
        break;
      case 1: 
        temp.numVal = num;
        map.insert(std::make_pair(key,temp));
        break;
      case 2:
        // temp.val = (void*)&dict;
        // temp.dict = dict;
        break;
      default :
        break;
      }
      //delete strKey;  
    }
    
    if (isKey) {
      isKey = false;
    } else 
      isKey = true;    
  }
  char str1[n-start+1];
  memcpy(str1, &data[start] , n-start);
  str1[n-start] = '\0'; 
  // std::cout << "############ " << str1 << std::endl;
  return (string("d") + string(str1) + "e");
}

// Use this map to print out all the required torrent info
void logTorrentInfo(std::unordered_map<std::string,BItem> fmap) {
  std::cout << "************   Torrent Info ****************" << std::endl;
  std::cout << "Keys available \t: " ;
  for (auto it = fmap.begin(); it != fmap.end(); ++it) {    
    std::cout << it->first << ",";
  }
  std::cout << "" << std::endl;
  std::cout << "Info Name \t: " << (char*)fmap.find("info.name")->second.val << std::endl;
  std::cout << "Announce Url \t: " << (char*)fmap.find("announce")->second.val << std::endl;
  std::cout << "Length \t\t: " << fmap.find("info.length")->second.numVal << std::endl;
  std::cout << "Piece Length \t: " << fmap.find("info.piece length")->second.numVal << std::endl;
  std::cout << "Date \t\t: " << fmap.find("creation date")->second.numVal << std::endl;
  std::cout << "********************************************" << std::endl;
}

void populateInfo(std::unordered_map<std::string,BItem> fmap , bt_info_t* t) {
  bzero(t->name , sizeof(t->name));
  BItem temp = fmap.find("info.name")->second;
  memcpy(t->name , temp.val,temp.size); //name of file
  t->piece_length = fmap.find("info.piece length")->second.numVal; //number of bytes in each piece 
  t->length = fmap.find("info.length")->second.numVal; //length of the file in bytes 
  t->num_pieces = (int)(ceil(t->length / t-> piece_length)); //number of pieces, computed based on above two values   
  
  temp = fmap.find("info")->second;
  // Info hash
  SHA1((unsigned char *) temp.val , temp.size, (unsigned char *) t->info_hash);
  
  /*********** - How to calculate sum of all hashes ?? Currently have only one piece hash in file ***/
  // temp = fmap.find("info.pieces")->second;
  // memcpy(t->piece_hashes,temp.val,temp.size); //pointer to 20 byte data buffers containing the sha1sum of each of the pieces
} 
int main3(int argc , char **argv) {
  string name = "moby_dick.txt.torrent";
  // string name = "bencode.cpp";
  long size = getFileSize(name);
  if (log_if(3.2))
    std::cout << "Torrent file Size " << size << std::endl;
  char data[size+1];
  getPacketFile (name, data, 0, size, true);
  data[size] = '\0';
  std::cout << data << std::endl;
  std::unordered_map<string,BItem> fmap;
  int i = 0 ;
  if (data[i] == 'd') {
    i++;
    parseDict(fmap,data,i,""); 
  } else {
    std::cout << "Error in format" << std::endl;
  }
  for (auto it = fmap.begin(); it != fmap.end(); ++it) {    
    std::cout << it->first << std::endl;
  }
  return 0;
}
#endif
