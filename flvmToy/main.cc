#include "flvm.hpp"
#include "unicode/brkiter.h"
#include "token.h"
#ifdef FLVM_TEST

void do_test()
{
#ifdef TOK_TEST
	tok_test1();
#endif
}

#define DO_TEST do_test()
#else 
#define DO_TEST 
#endif

int main(int argc, char** argv) {
	DO_TEST;
	printf("flvm-toy-main\n");
	return 0;
}