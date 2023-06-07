#pragma once
#include "common.h"
#include "token.h"

enum IRNodeTag {
	IRTag_Id,
	IRTag_Num,
	IRTag_Add,
};

class IRNode
{
public:
	IRNode() :IRNode(0, {0,0}) {}
	IRNode(token_t _tok, position_t _s) 
		: _m_tok(_tok), _m_begin_pos(_s), _m_end_pos({0,0}) {};

	virtual IRNodeTag tag() = 0;

public:
	token_t		 token()			{ return _m_tok; }
	position_t start_loc()	{ return _m_begin_pos; }
	position_t end_loc()		{ return _m_end_pos; }
	void set_tok(token_t tok)					{ _m_tok = tok; }
	void set_end_loc(position_t _e)		{ _m_end_pos = _e; }
	void set_start_loc(position_t _s) { _m_begin_pos = _s; }
protected:
	position_t _m_begin_pos;
	position_t _m_end_pos;
	token_t		 _m_tok;
};

#define IRNode_Impl(Tag) \
public: \
	virtual IRNodeTag tag() override {\
		return Tag;\
	}
	
// id node
class IR_Id: public IRNode
{
	IRNode_Impl(IRTag_Id)
public:
	IR_Id(token_t tok, position_t _s): IRNode(tok, _s) {}
private:
};

// number node
class IR_Num : public IRNode
{
	IRNode_Impl(IRTag_Num)
public:
	IR_Num(token_t tok, position_t _s): IRNode(tok, _s){}
	bool is_int()		{ return token_is_kind(token(), TokInt); }
	bool is_float() { return token_is_kind(token(), TokFloat); }
};

// binary node
class IR_BinOp: public IRNode
{
public:
	IR_BinOp(token_t tok, position_t _s, IRNodeTag _tag, IRNode* lhs, IRNode* rhs)
		: IRNode(tok, _s), _m_tag(_tag), _m_lhs(lhs), _m_rhs(rhs) {}
	IR_BinOp(token_t tok, position_t _s, IRNodeTag _tag)
		: IR_BinOp(tok, _s, _tag, nullptr, nullptr) {}
	virtual IRNodeTag tag() override { return _m_tag; }
	IRNode* lhs() const { return _m_lhs; }
	IRNode* rhs() const { return _m_rhs; }
private:
	IRNode* _m_lhs;
	IRNode* _m_rhs;
	IRNodeTag _m_tag;
};

