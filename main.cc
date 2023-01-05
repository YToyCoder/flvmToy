#include <iostream>

enum Instruction {
  iconst_1 = 0x01,
  iconst_2 = 0x01,
  iconst_3 = 0x01,
  iconst_4 = 0x01,
};

int main(int argc, char const *argv[])
{
  std::cout << "hello, this is flvm" << std::endl;
  return 0;
}
