#include "ir.h"

IRNode* IR_BinOp::accept(IRVisitor& visitor)
{
	IR_BinOp* self = this;
	if (_m_lhs != nullptr)
	{
		self = set_lhs(_m_lhs->accept(visitor));
	}
	if (_m_rhs != nullptr && self != nullptr)
	{
		self = set_rhs(_m_rhs->accept(visitor));
	}
	return nullptr != self ? visitor.visit(self) : visitor.visit(this);
}

IR_BinOp* IR_BinOp::set_lhs(IRNode* _lhs)
{
	if (_lhs == _m_lhs || _lhs == nullptr)
		return this;
	return new IR_BinOp(token(), end_loc(), tag(), _lhs, _m_rhs);
}

IR_BinOp* IR_BinOp::set_rhs(IRNode* _rhs)
{
	if (_rhs == _m_rhs || _rhs == nullptr)
		return this;
	return new IR_BinOp(token(), end_loc(), tag(), _m_lhs, _rhs);
}
