#ifndef flvm_toy_test
#define flvm_toy_test
#include <iostream>

void assert_true(bool test_res, const char *msg){
  if(!test_res)
    std::cout << msg << std::endl;
}
#endif