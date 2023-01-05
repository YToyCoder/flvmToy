#include <ctime>
#include <stdio.h>
#include "../MurmurHash2.cc"

int main(int argc, char const *argv[])
{
  time_t seed1 = time(nullptr);
  time_t seed2 = time(nullptr);
  const char a[] = "1234";
  printf("seed %x hash %x\n", seed1, MurmurHash64A(a,4,seed1));
  printf("seed %x hash %x\n", seed2, MurmurHash64A(a,4,seed2));
  return 0;
}
