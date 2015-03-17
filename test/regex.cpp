#include <regex>
#include <iostream>
#include <string>

using namespace std;

bool isMatch(string str,string r) {  
  return regex_match(str , regex(r)) ? true : false;
};

string* split(string str , string spl) {
  return NULL; 
};


int main(int argc ,char **argv){
  cout<<"Hello win";  
  cout<<isMatch("HTTP 1.1: dasdasd" , "HTTP.*");

  const std::regex pattern("hello ([0-9]+)");

  // the source text
  std::string text = "hello 1, hello 2, hello 17, and done!";

  const std::sregex_token_iterator end;
  for (std::sregex_token_iterator i(text.cbegin(), text.cend(), pattern);
       i != end;
       ++i) {
    std::cout << *i << std::endl;
  }

  // regex_search("a=1,b=2,c=3",m,regex(","));
  // for (auto x : m ){
  //   cout<<x <<" ";
  // }
  // // regular expression
  // if (regex_match ("subject", regex("(sub)(.*)") ))
  //   cout << "string literal matched\n";

  // const char cstr[] = "subject";
  // string s ("subject");
  // regex e ("(sub)(.*)");

  // if (regex_match (s,e))
  //   cout << "string object matched\n";

  // if ( regex_match ( s.begin(), s.end(), e ) )
  //   cout << "range matched\n";

  // cmatch cm;    // same as match_results<const char*> cm;
  // regex_match (cstr,cm,e);
  // cout << "string literal with " << cm.size() << " matches\n";

  // smatch sm;    // same as match_results<string::const_iterator> sm;
  // regex_match (s,sm,e);
  // cout << "string object with " << sm.size() << " matches\n";

  // regex_match ( s.cbegin(), s.cend(), sm, e);
  // cout << "range with " << sm.size() << " matches\n";

  // // using explicit flags:
  // regex_match ( cstr, cm, e, regex_constants::match_default );

  // cout << "the matches were: ";
  // for (unsigned i=0; i<sm.size(); ++i) {
  //   cout << "[" << sm[i] << "] ";
  // }

  // cout << endl;
  return 0;
}
