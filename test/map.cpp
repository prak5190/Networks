#include <unordered_map>
#include <iostream>
#include <inttypes.h>
using namespace std;

struct fileInfo { 
  string filename;
  uint64_t size;
};

int main(int argc, char **argv)
{
  unordered_map<string, fileInfo> fileInfoMap;
  auto end = fileInfoMap.end();
  fileInfo a;
  a.filename = "dasdasd";
  a.size = 11111;

  fileInfoMap["dasdasd"] = a;
  if ( fileInfoMap.find("dasd") == end){
    cout<<"NUll is cool";
  }
  // fileInfo k = fileInfoMap.find("dasd");
  // cout<<k.filename;
  // if (fileInfoMap.find("d1")) {
  //   cout<<"Found somethis";
  // }
  unordered_map<string, int> m;
  m["hello"] = 23;

  // check if key is present
  if (m.find("world") != m.end())
    cout << "unordered_map contains key world!\n";
  // retrieve
  cout << m["hello"] << '\n';
  unordered_map<string, int>::iterator i = m.find("hello");


  cout << "Key: " << i->first << " Value: " << i->second << '\n';
  return 0;
}
