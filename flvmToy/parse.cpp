#include "parse.h"
#include "lex.h"

/* tokens */
void Parser::try_fill_cache()
{
	if (_m_tok_cache.empty() && _m_lex.has_token())
	{
		for (int i = 0; i < Parser_Token_Cache_Limit; i++)
		{
			if (!_m_lex.has_token()) break;
			_m_tok_cache.push_back(_m_lex.next_token());
		}
	}
}

token_t Parser::token()
{
	try_fill_cache();
	return _m_tok_cache.empty() ? InvalidTok : _m_tok_cache.front();
}

token_t Parser::next_tok()
{
	try_fill_cache();
	if (_m_tok_cache.empty())
		return InvalidTok; 
	token_t tok = _m_tok_cache.front();
	_m_tok_cache.pop_front();
	return tok;
}

bool Parser::has_tok()
{
	return !_m_tok_cache.empty() || _m_lex.has_token();
}

IRNode* Parser::parse()
{
	return has_tok() ? parsing_binary() : nullptr ;
}

IRNode* Parser::parsing_num()
{
	token_t t = next_tok();
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
	token_t t = next_tok();
	return new IR_Id(t, tok_foffset(t), tok_end(t));
}

IRNode* Parser::parsing_literal()
{
	TokenKind tk = token_kind(token());
	switch (tk)
	{
	case TokId:
		return parsing_id();
	case TokInt:
	case TokFloat:
		return parsing_num();
	}
	printf("not support token kind for literal : %s\n", tk_to_str(tk).c_str());
	throw std::exception("not support kind literal");
}
IRNode* Parser::parsing_binary()
{
	IRNode* lhs = parsing_literal();
	token_t operator_tok;
	while (has_tok() && token_is_operator(operator_tok = token()))
	{
		next_tok();
		if (!has_tok())
		{
			throw std::exception("parsing binary node exception[ has no right-hand side expression ]");
		}
		IRNode* rhs = parsing_literal();
		lhs = new IR_BinOp(
			operator_tok, 
			lhs->start_loc(), 
			token_kind_to_tag(token_kind(operator_tok)),
			lhs, rhs);
	}
	return lhs;
}
