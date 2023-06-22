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

IRNode* IR_Decl::accept(IRVisitor& visitor)
{
	return visitor.visit(set_init(m_init->accept(visitor)));
}

IrNode_Set_Method_Impl(Decl, init)

IRNode* IR_Block::accept(IRVisitor& visitor)
{
	return visitor.visit(proc_ls(visitor));
}

// *********************************** IR_If *********************************

IrNode_Set_Method_Impl(If, test)
IrNode_Set_Method_Impl(If, success)
IrNode_Set_Method_Impl(If, failed)

IRNode* IR_If::accept(IRVisitor& visitor)
{
	auto node = this;
	// nullptr ??
	node = node->set_test(node->m_test->accept(visitor));
	node = node->set_success(node->m_success->accept(visitor));
	if(node && node->m_failed){
		node = node->set_failed(node->m_failed->accept(visitor));
	}
	return node == nullptr ? this : node;
}

// *********************************** IR_STRING ******************************
std::string IR_String::stringify(IRNode* ir)
{
	ir->accept(*this);
	return _m_ss.str();
}

IRNode* IR_String::visit(IR_Id* exp)
{
	if (exp != nullptr)
		_m_ss << "[Id]";
	return exp;
}

IRNode* IR_String::visit(IR_Str* string)
{
	if(string != nullptr)
		_m_ss << "\"" << "string" << "\"";
	return string;
}

IRNode* IR_String::visit(IR_Num* exp)
{
	if (exp != nullptr) {
		_m_ss 
			<< (exp->is_float() ? "D(" + std::to_string(exp->get_db()) +")" : "")
			<< (exp->is_int() ? "I(" + std::to_string(exp->get_int()) + ")" : "");
  }
	return exp;
}

IRNode* IR_String::visit(IR_Cast* exp)
{
	if (nullptr != exp) {
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
		_m_ss << " <";
		exp->lhs()->accept(*this);
		_m_ss << "[" << IRTag_to_string(exp->tag()) << "]";
		exp->rhs()->accept(*this);
		_m_ss << "> ";
	}
	return exp;
}

IRNode* IR_String::visit(IR_Decl* decl)
{
	if (nullptr != decl)
	{
		_m_ss << "<let?" << decl->id() << "=";
		decl->init()->accept(*this);
		_m_ss << ">";
	}
	return decl;
}

IRNode* IR_String::visit(IR_Block* block)
{
	if (nullptr != block)
	{
		_m_ss << "{" << std::endl;
		for (const auto& it : block->list())
		{
			it->accept(*this);
			_m_ss << std::endl;
		}
		_m_ss << "}" << std::endl;
	}
	return block;
}

// TODO:?
IRNode* IR_String::visit(IR_If* stmt_if)
{
	return stmt_if;
}
