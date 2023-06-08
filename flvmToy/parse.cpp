#include "parse.h"


IRNode* Parser::parse()
{
	return nullptr;
}

IRNode* Parser::parsing_num()
{
	token_t t = _m_lex.next_token();
	IR_Num* node = new IR_Num(t, tok_foffset(t), tok_end(t));
	node->_m_num;
	return node;
}

IRNode* Parser::parsing_id()
{
	token_t t = _m_lex.next_token();
	return new IR_Id(t, tok_foffset(t), tok_end(t));
}