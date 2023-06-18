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
		Parser parser("new.txt");
		Context ct;
		if (!parser.init(&ct))
		{
			printf("inittialize failed \n");
			return;
		}
		IRNode* node = parser.parse();
		IR_String irs;
		std::cout << irs.stringify(node) << std::endl;;
		TypeConvert tc(&ct);
		auto n = tc.convert(node);
		irs.clear();
		//std::cout << irs.to_string() << std::endl;
		std::cout << irs.stringify(n) << std::endl;
		CodeGen cg(&ct);
		cg.build(n);
		auto method = cg.get_method();
		method->to_string();
		FlSExec exec;
		exec.exec(method.get());
	}

#endif
}

#define DO_TEST do_test()
#else 
#define DO_TEST 
#endif

int main(int argc, char** argv) {
	printf("%d \n", __cplusplus);
	set_instr_map();
	DO_TEST;
	return 0;
}