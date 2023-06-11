#include "flvm.hpp"
#include "unicode/brkiter.h"
#include "unicode/ustream.h"
#include "parse.h"
#include "token.h"
#include "lex.h"
#include <stdio.h>

#define FLVM_TEST
#ifdef FLVM_TEST

void do_test()
{
#ifdef TOK_TEST
	{
		tok_test1();
		Lex lex("new.txt");
		lex.init();
		while (lex.has_token()) {
			token_t t = lex.next_token();
			printf(">> %s\n", token_to_str(t).c_str());
			std::cout << lex.token_string(t) << std::endl;
		}
	}

	{
		Parser parser("new.txt");
		IRNode* node = parser.parse();
		IR_String irs;
		std::cout << irs.stringify(node) << std::endl;;
		TypeConvert tc;
		auto n = tc.convert(node);
		irs.clear();
		std::cout << irs.to_string() << std::endl;
		std::cout << irs.stringify(n) << std::endl;
		CodeGen cg;
		cg.build(n);
		auto method = cg.get_method();
		std::cout << method->to_string() << std::endl;
		FlSExec exec;
		exec.exec(method.get());
		printf("parsing finish\n");
	}

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