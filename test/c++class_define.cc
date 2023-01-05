#include <iostream>
#include <stdio.h>

class Declare;
typedef Declare *ThePointer;

union MUseNode {
  union MUseNode * _link;
  char user_data[1];
};

typedef union MUseNode Node;

int main(int argc, char const *argv[])
{
  char ca[1];
  Node *data;
  void* ptr = data;
  char c;
  printf("size of %d\n", sizeof(ThePointer));
  printf("size of wchar_t %d\n", sizeof(wchar_t));
  printf("size of 1 %d\n", sizeof(ca));
  printf("size of 2 %d\n", sizeof(MUseNode));
  printf("size of 3 %d\n", sizeof data->user_data);
  printf("size of 4 %d\n", sizeof data->_link);
  return 0;
}
