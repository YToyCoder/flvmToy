#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include "../flvm.cc"

std::string bin_path(){
  char *path = getcwd(NULL,0);
  std::string dir = std::string(path);
  return dir + "/test/bin/"; 
}

struct TTag {
  enum{
    Int = 1
  };
};

void load(const char* filename){
  auto the_name = bin_path() + std::string(filename);
  printf("load file (%s)\n", the_name.c_str());
  std::ifstream file(the_name, std::ios::in | std::ios::binary);
  // magic number >> F1
  uint8_t mn;
  file.read((char *)&mn, 1);
  // version
  printf("magic number is %x\n", mn);
  uint16_t cp_size;
  file.read((char *)&cp_size, 2);
  printf("const pool size %x\n", cp_size);
  uint8_t tag;
  for(int i=0; i<cp_size; i++){
    file.read((char *)&tag, 1);
    switch(tag){
      case TTag::Int:
        FlInt v;
        file.read((char *)&v, 8);
        printf("one int const %d\n", v);
        break;
    }
  }
  file.close();
}

int main(int argc, char const *argv[])
{
  load("hello");
  return 0;
}
