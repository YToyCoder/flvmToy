#include "parse.h"


IRNode* Parser::parse()
{
	return nullptr;
}

IRNode* Parser::parsing_num()
{
	token_t t = _m_lex.next_token();
	uint16_t row  = token_row(t);
	uint16_t scol = token_scol(t);
	uint16_t ecol = token_ecol(t);
	IR_Num* node = new IR_Num(t, position_t{row, scol}, position_t{row, ecol});
	node->_m_num;
	return node;
}

IRNode* Parser::parsing_id()
{
	token_t t = _m_lex.next_token();
	uint16_t row  = token_row(t);
	uint16_t scol = token_scol(t);
	uint16_t ecol = token_ecol(t);
	return new IR_Id(t, position_t{row, scol}, position_t{row, ecol});
}