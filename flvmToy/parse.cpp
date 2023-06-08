#include "parse.h"
#include "lex.h"


IRNode* Parser::parse()
{
	return nullptr;
}

IRNode* Parser::parsing_num()
{
	token_t t = _m_lex.next_token();
	IR_Num* node = new IR_Num(t, tok_foffset(t), tok_end(t));
	Lex::UStr str = _m_lex.token_string(t);
	std::string u8s;
	str.toUTF8String(u8s);
	switch (token_kind(t)) {
	case TokInt:
		node->_m_num.i_value = std::stoi(u8s);
		break;
	case TokFloat:
		node->_m_num.db_value= std::stod(u8s);
		break;
	}
	return node;
}

IRNode* Parser::parsing_id()
{
	token_t t = _m_lex.next_token();
	return new IR_Id(t, tok_foffset(t), tok_end(t));
}