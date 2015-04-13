#include "common.cpp"
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
  if (log(3))
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
  void* val;
  int type; // 0 - Str , 1 - Num , 2 - dict , 3 -list
};


void parseDict(std::unordered_map<std::string,BItem> &map,char* data, int &n,string prefix) {
  long size = 0;
  bool isKey = true;
  char* strKey,*str;  
  int type = 0; // 0 - Str , 1 - Num , 2 - dict , 3 -list
  if (log(5.1))
    std::cout << "Torrent String "<<data << std::endl;
  while(data[n]!='\0') {    
    // Represents string 
    long num;
    if(log (3.2))
      std::cout << "MEta "<< data[n] << std::endl;
    if (data[n] >= 48 && data[n] < 58) {
      size = parseSize(data,n);
      if (log(3))
        std::cout << "Size is " << size << " : "<< n << std::endl;
      str = new char[size+1];
      bzero(str,size+1);
      str[size] = '\0';
      copyString(data,n,str,size);
      if (log(3))
        std::cout << "String : "<< str << std::endl;
      if (isKey) {
        strKey = str;
      }
      type = 0;
    } else if (data[n] == 'i') {
      n++;
      num = parseNum(data,n);      
      if (log(3))
        std::cout << "Number " << num << std::endl;
      type = 1;
    } else if (data[n] == 'd') {        
      n++;
      parseDict(map,data,n, prefix + string(strKey) + ".");
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
      if (log(3.5))
        std::cout << "Key : "<< strKey <<" - ";
      string key = prefix +  string(strKey);
      BItem temp;
      temp.type = type;
      switch (type) {
      case 0 : 
        if(log(3.5))
          std::cout << str << std::endl;
        temp.val = str;
        map.insert(std::make_pair(key,temp));
        break;
      case 1: 
        if (log(3.5))
          std::cout << num << std::endl;
        temp.val = (void*)&num;
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
  std::cout << "Date \t: " << (long)fmap.find("creation date")->second.val << std::endl;
  std::cout << "********************************************" << std::endl;
}

int main2(int argc , char **argv) {
  string name = "moby_dick.txt.torrent";
  // string name = "bencode.cpp";
  long size = getFileSize(name);
  if (log(3.2))
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
