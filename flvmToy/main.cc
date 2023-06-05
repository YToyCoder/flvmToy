#include "flvm.hpp"
#include "unicode/brkiter.h"
#include "token.h"

int main(int argc, char** argv) {
#ifdef TOK_TEST
	tok_test1();
#endif
	printf("flvm-toy-main\n");
	return 0;
}