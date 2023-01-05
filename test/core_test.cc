#include <iostream>
#include "../flvmToy.hpp"
#include "flvmt.h"

int main(int argc, char const *argv[])
{
  /* code */
  flvm::core::FlStack<int> istk(10);
  assert_true(istk.isEmpty(), "init stack should be empty");
  istk.push(10);
  assert_true(istk.pop() == 10, "10");
  assert_true(istk.isEmpty(), "stack should be empty");
  return 0;
}
