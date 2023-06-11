#pragma once
#include "lex.h"
#include "ir.h"
#include "flvm.hpp"
#include <list>
#include <set>
#include <map>
#include <functional>

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
	IRNode* parsing_mul();
	IRNode* parsing_add();
	IRNode* binary_parsing_proccess(
		std::function<bool(token_t)> fn_token_is_right_kind,
		std::function<IRNode*()> fn_xhand_side_parsing);

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

inline CodeGenType better_type(CodeGenType ta, CodeGenType tb)
{
	return max(ta, tb);
}

class TypeConvert : public IRVisitor
{
public:
	IRNode* convert(IRNode* ir);
private:
	virtual IRNode* visit(IR_Num* num) override;
	virtual IRNode* visit(IR_BinOp* bin) override;
	virtual IRNode* visit(IR_Id* id) override
	{
		throw std::runtime_error("not support id code gen");
	}
	virtual IRNode* visit(IR_Cast* id) override;// { return id; }
	inline void push_t(CodeGenType t) { t_stk.push_back(t); }
	inline CodeGenType pop_t() {
		auto t = t_stk.back();
		t_stk.pop_back();
		return t;
	}

	std::list<CodeGenType> t_stk;
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
	inline CodeGen* add_instr(uint8_t instr) { _m_builder.append(instr); return this; }
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
	inline CodeGenType top_t() { return t_stk.front(); }
	inline void push_t(CodeGenType t) 
	{ 
		t_stk.push_back(t); 
		set_max_stk_size();
	}

	inline void set_max_stk_size()
	{
		if (t_stk.size() > max_stk_size)
		{
			max_stk_size = t_stk.size();
		}
	}

	// 从本地变量表中申请一个新的本地变量表
	inline uint8_t alloc_local(unistr_t name = "~temp~name")
	{
		if (!m_unused_local.empty())
		{
			uint8_t n_local = m_max_local++;
		}
	}

	// type stack
	std::list<CodeGenType> t_stk; // type stack
	size_t max_stk_size;
	// local info
	uint8_t m_max_local; // init 0
	std::set<uint8_t> m_unused_local;
	std::map<unistr_t, uint8_t> m_local_name_map; // string map to local index
	FlMethodBuilder _m_builder;
};
