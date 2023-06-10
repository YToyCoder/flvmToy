#include "ir.h"

IRNode* IR_BinOp::accept(IRVisitor& visitor)
{
	IR_BinOp* self = this;
	if (_m_lhs.get() != nullptr)
	{
		self = set_lhs(_m_lhs->accept(visitor));
	}
	if (_m_rhs.get() != nullptr)
	{
		self = set_rhs(_m_rhs->accept(visitor));
	}
	return nullptr != self ? visitor.visit(self) : visitor.visit(this);
}

IR_BinOp* IR_BinOp::set_lhs(IRNode* _lhs)
{
	if (_lhs == _m_lhs.get())
		return this;
	auto self = new IR_BinOp(*this);
	self->_m_lhs = std::shared_ptr<IRNode>(_lhs);
	return self;
}

IR_BinOp* IR_BinOp::set_rhs(IRNode* _rhs)
{
	if (_rhs == _m_rhs.get()) 
		return this;
	auto self = new IR_BinOp(*this);
	self->_m_rhs = std::shared_ptr<IRNode>(_rhs);
	return self;
}
