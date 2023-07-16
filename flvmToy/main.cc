#include "flvm.hpp"
#include "unicode/brkiter.h"
#include "unicode/ustream.h"
#include "parse.h"
#include "token.h"
#include "lex.h"
#include <stdio.h>

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
	}

#endif
}

int main(int argc, char** argv) {
	printf("%d \n", __cplusplus);
	set_instr_map();
	if(argc <= 1)
		return 0;
	printf("%s\n", argv[1]);
#if 0
	Lex lex(argv[1]);
	Context c;
	lex.init(&c);
	while(lex.has_token()) {
		std::cout << tk_to_str(lex.next_token()) << std::endl;
	};
#else 
	Parser parser(argv[1]);
	Context c;
	if (!parser.init(&c))
	{
		printf("inittialize failed \n");
		return 0;
	}
	IRNode* node = parser.parse();
	IR_String irs;
	std::cout << irs.stringify(node) << std::endl;;
	TypeConvert tc(&c);
	auto n = tc.convert(node);
	irs.clear();
	std::cout << irs.stringify(n) << std::endl;
	CodeGen cg(&c);
	FnProto* proto = cg.buildProto(n);
	proto->to_string();
#endif
	return 0;
}