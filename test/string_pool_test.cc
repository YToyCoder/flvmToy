#include "../flvm.cc"

FlString* ofConstString(const char *p, FlStringContsPool *pool){
  int size = 0;
  char *walk = const_cast<char *>(p);
  while(*walk != '\0'){
    size++;
    walk++;
  }
  return pool->ofFlstring(p, size);
}

int main(int argc, char const *argv[])
{
  FlStringContsPool pool;
  auto s1 = ofConstString("hello", &pool);
  auto s2 = ofConstString("hello", &pool);
  printf("%s\n", s1 == s2 ? "true" : "false");
  printf("%s\n", s2->length() == 5 ? "true" : "false");
  return 0;
}
