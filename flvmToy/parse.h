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

enum CodeGenType
{
	CodeGen_I, // int
	CodeGen_D, // double
};

class CodeGen: public IRVisitor
{
public:
	void build(IRNode* ir);
private:
	virtual IRNode* visit(IR_Num* num) override;
	virtual IRNode* visit(IR_Id* id) override
	{
		throw std::runtime_error("not support id code gen");
	}
	virtual IRNode* visit(IR_BinOp* bin) override;
	CodeGenType gen_num(IR_Num* ir);
	CodeGenType gen_bin(IR_BinOp* ir);

// code append operation
	inline CodeGen* add_instr(uint8_t instr) { _m_builder.append(instr); }
	inline void add_int16_to_code(int16_t _i)
	{
		uint8_t* byte = (uint8_t*)(&_i);
		_m_builder.append(*byte);
		_m_builder.append(*(byte + 1));
	}
// type operation
	inline CodeGenType pop_t()
	{
		CodeGenType t = t_stk.front();
		t_stk.pop_front();
		return t;
	}
	inline void push_t(CodeGenType t) { t_stk.push_back(t); }

	inline void set_max_stk_size()
	{
		if (t_stk.size() > max_stk_size)
		{
			max_stk_size = t_stk.size();
		}
	}

	std::list<CodeGenType> t_stk; // type stack
	size_t max_stk_size;
	FlMethodBuilder _m_builder;
};
