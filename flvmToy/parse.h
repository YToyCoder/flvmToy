#pragma once
#include "lex.h"
#include "ir.h"
#include "unicode/ustream.h"
#include "flvm.hpp"
#include <list>
#include <set>
#include <map>
#include <iostream>
#include <functional>

#define Parser_Token_Cache_Limit 2

class Parser
{
public:
	Parser(const std::string& filename) :_m_lex(filename) {}
	Parser(): Parser("") {}
	IRNode* parse();
	bool init(Context* c) { return _m_lex.init(c); }
protected:
	IRNode* parsing_num();
	IRNode* parsing_id();
	IRNode* parsing_literal();
	IRNode* parsing_mul();
	IRNode* parsing_add();
	IRNode* binary_parsing_proccess(
		std::function<bool(token_t)> fn_token_is_right_kind,
		std::function<IRNode*()> fn_xhand_side_parsing);

	IRNode* parsing_decl();

	//				token				//
	inline token_t token(); 
	inline token_t next_tok();
	inline token_t next_tok_must(TokenKind tk);
	inline bool		has_tok(); 
	inline void		try_fill_cache();
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
class NameTypeMap
{
public:
	NameTypeMap(Context* c): m_context(c) {}
protected:
	void decl(unistr_t n, unistr_t t)
	{
		auto it = m_name_type_map.find(n);
		if (it != m_name_type_map.end())
		{
			std::cout << "variable " << n << " have been declared " << std::endl;
			throw std::exception("redeclare for variable name");
		}
		m_name_type_map.insert(std::make_pair(n, t));
	}

	CodeGenType variable_t(unistr_t n)
	{
		auto it = m_name_type_map.find(n);
		if (it == m_name_type_map.end())
		{
			std::cout << "variable " << n << " not declared" << std::endl;
			throw std::exception("variable not declared");
		}
		unistr_t type_name = it->second;
		if (type_name == NodeInt)
		{
			return CodeGen_I;
		}else if (type_name == NodeDouble)
		{
			return CodeGen_D;
		}
		else
		{
			std::cout << "not support type for " << type_name << std::endl;
			throw std::exception("not support type");
		}
	}

	CodeGenType id_node_type(const IR_Id* id)
	{
		unistr_t str;
		if (!m_context->find_tok_str(id->token(), str))
		{
			throw std::exception("could not find id node token str");
		}
		return variable_t(str);
	}

	void find_node_str(const IR_Id* id, unistr_t& str)
	{
		if (!m_context->find_tok_str(id->token(), str))
		{
			throw std::exception("could not find id node token str");
		}
	}


	std::map<unistr_t, unistr_t> m_name_type_map;
	Context* m_context;
};
class TypeConvert : public IRVisitor, public NameTypeMap
{
public:
	TypeConvert(Context* c) : NameTypeMap(c) { }
	IRNode* convert(IRNode* ir);
private:
	virtual IRNode* visit(IR_Num* num) override;
	virtual IRNode* visit(IR_BinOp* bin) override;
	virtual IRNode* visit(IR_Id* id) override;
	virtual IRNode* visit(IR_Cast* cast) override;
	virtual IRNode* visit(IR_Decl* decl) override;
	inline void push_t(CodeGenType t) { t_stk.push_back(t); }
	inline CodeGenType pop_t() {
		auto t = t_stk.back();
		t_stk.pop_back();
		return t;
	}
	std::list<CodeGenType> t_stk;
};

class CodeGen: protected IRVisitor, protected NameTypeMap
{
public:
	CodeGen(Context* c): NameTypeMap(c), m_max_local(0), max_stk_size(0) { }
	void build(IRNode* ir);
	sptr_t<FlMethod> get_method();
private:
	IR_Visitor_Impl_Decl()
	
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
		CodeGenType t = t_stk.back();
		t_stk.pop_back();
		return t;
	}
	inline CodeGenType top_t() { return t_stk.back(); }
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
	inline uint8_t local_for_name(unistr_t name)
	{
		auto it = m_local_name_map.find(name);
		if (it != m_local_name_map.end())
			return it->second;
		uint8_t local = m_max_local++;
		m_local_name_map.insert(std::make_pair(name, local));
		return local;
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
