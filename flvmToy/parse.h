#pragma once
#include "lex.h"
#include "ir.h"
#include "flvm.hpp"
#include <list>

#define Parser_Token_Cache_Limit 2

class Parser
{
public:
	Parser(const std::string& filename) :_m_lex(filename) { _m_lex.init(); }
	Parser(): Parser("") {}
	IRNode* parse();
protected:
	IRNode* parsing_num();
	IRNode* parsing_id();
	IRNode* parsing_literal();
	IRNode* parsing_binary();

	//				token				//
	token_t token(); 
	token_t next_tok();
	bool		has_tok(); 
	void		try_fill_cache();
private:
	std::list<token_t> _m_tok_cache;
	Lex _m_lex;
};

