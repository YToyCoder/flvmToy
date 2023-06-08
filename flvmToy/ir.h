#pragma once
#include "common.h"
#include "token.h"

enum IRNodeTag {
	IRTag_Id = 1,
	IRTag_Num,
	IRTag_Add,
	IRTag_Sub,
	IRTag_Mul,
	IRTag_Div,
};

#define TOKEN_KIND_RET_TAG(ID) case Tok##ID: return IRTag_##ID

inline IRNodeTag token_kind_to_tag(TokenKind tk)
{
	switch (tk)
	{
		TOKEN_KIND_RET_TAG(Add);
		TOKEN_KIND_RET_TAG(Sub);
		TOKEN_KIND_RET_TAG(Mul);
		TOKEN_KIND_RET_TAG(Div);
	}
	throw std::exception("token kind to tag error");
}

class IRNode
{
public:
	IRNode() :IRNode(0, 0) {}
	IRNode(token_t _tok, uint32_t _s) 
		: IRNode(_tok,_s, 0) {};
	IRNode(token_t _tok, uint32_t _s, uint32_t _e) 
		: _m_tok(_tok), _m_begin_pos(_s), _m_end_pos(_e) {};

	virtual IRNodeTag tag() = 0;

public:
	token_t		 token()			{ return _m_tok; }
	uint32_t start_loc()	{ return _m_begin_pos; }
	uint32_t end_loc()		{ return _m_end_pos; }
	void set_tok(token_t tok)				{ _m_tok = tok; }
	void set_end_loc(uint32_t _e)		{ _m_end_pos = _e; }
	void set_start_loc(uint32_t _s) { _m_begin_pos = _s; }
protected:
	uint32_t _m_begin_pos;
	uint32_t _m_end_pos;
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
	IR_Id(token_t tok, uint32_t _s)
		: IRNode(tok, _s) {}
	IR_Id(token_t tok, uint32_t _s, uint32_t _e)
		: IRNode(tok, _s, _e){}
private:
};

// number node
class IR_Num : public IRNode
{
	IRNode_Impl(IRTag_Num)
		friend class Parser;
public:
	IR_Num(token_t tok, uint32_t _s): IRNode(tok, _s), _m_num(){}
	IR_Num(token_t tok, uint32_t _s, uint32_t _e)
		: IRNode(tok, _s, _e), _m_num(){}
	bool is_int()		{ return token_is_kind(token(), TokInt); }
	bool is_float() { return token_is_kind(token(), TokFloat); }
	int			get_int() { return _m_num.i_value; }
	double	get_db()	{ return _m_num.db_value; }
private:
	union {
		double db_value;
		int		 i_value;
	} _m_num;
};

// binary node
class IR_BinOp: public IRNode
{
public:
	IR_BinOp(token_t tok, uint32_t _s, IRNodeTag _tag, IRNode* lhs, IRNode* rhs)
		: IRNode(tok, _s), _m_tag(_tag), _m_lhs(lhs), _m_rhs(rhs) {}
	IR_BinOp(token_t tok, uint32_t _s, IRNodeTag _tag)
		: IR_BinOp(tok, _s, _tag, nullptr, nullptr) {}
	virtual IRNodeTag tag() override { return _m_tag; }
	IRNode* lhs() const { return _m_lhs; }
	IRNode* rhs() const { return _m_rhs; }
private:
	IRNode* _m_lhs;
	IRNode* _m_rhs;
	IRNodeTag _m_tag;
};

