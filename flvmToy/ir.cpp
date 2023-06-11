#include "ir.h"
#include "unicode/ustream.h"
#include <stdio.h>

IRNode* IR_BinOp::accept(IRVisitor& visitor)
{
	IR_BinOp* self = this;
	if (_m_lhs.get() != nullptr)
	{
		self = set_lhs(_m_lhs->accept(visitor));
	}
	else {
		printf("error, binary_operator node lhs is nullptr\n");
	}
	if (self->_m_rhs.get() != nullptr)
	{
		self = self->set_rhs(self->_m_rhs->accept(visitor));
	}
	else {
		printf("error, binary_operator node rhs is nullptr\n");
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

std::string IR_String::stringify(IRNode* ir)
{
	ir->accept(*this);
	return _m_ss.str();
}

IRNode* IR_String::visit(IR_Id* exp)
{
	if (exp != nullptr)
	{
		_m_ss << "[Id]";
	}
	return exp;
}

IRNode* IR_String::visit(IR_Num* exp)
{
	if (exp != nullptr)
	{
		_m_ss 
			<< (exp->is_float() ? "D(" + std::to_string(exp->get_db()) +")" : "")
			<< (exp->is_int() ? "I(" + std::to_string(exp->get_int()) + ")" : "");
	}
	return exp;
}

IRNode* IR_String::visit(IR_Cast* exp)
{
	if (nullptr != exp)
	{
		_m_ss << "<C|";
		exp->cast_from()->accept(*this);
		_m_ss << ":" << exp->cast_to() << ">";
	}
	return exp;
}

IRNode* IR_String::visit(IR_BinOp* exp)
{
	if (nullptr != exp)
	{
		_m_ss << "<";
		exp->lhs()->accept(*this);
		_m_ss << "[" << IRTag_to_string(exp->tag()) << "]";
		exp->rhs()->accept(*this);
		_m_ss << ">";
	}
	return exp;
}
