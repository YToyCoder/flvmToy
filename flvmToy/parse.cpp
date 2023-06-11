#include "parse.h"
#include "lex.h"
#include <functional>

/* tokens */
void Parser::try_fill_cache()
{
	if (_m_tok_cache.empty() && _m_lex.has_token())
	{
		for (int i = 0; i < Parser_Token_Cache_Limit; i++)
		{
			if (!_m_lex.has_token()) break;
			_m_tok_cache.push_back(_m_lex.next_token());
		}
	}
}

token_t Parser::token()
{
	try_fill_cache();
	return _m_tok_cache.empty() ? InvalidTok : _m_tok_cache.front();
}

token_t Parser::next_tok()
{
	try_fill_cache();
	if (_m_tok_cache.empty())
		return InvalidTok; 
	token_t tok = _m_tok_cache.front();
	_m_tok_cache.pop_front();
	return tok;
}

bool Parser::has_tok()
{
	return !_m_tok_cache.empty() || _m_lex.has_token();
}

IRNode* Parser::parse()
{
	if (has_tok())
		return parsing_add();
	return nullptr;
}

IRNode* Parser::parsing_num()
{
	token_t t = next_tok();
	IR_Num* node = new IR_Num(t, tok_foffset(t), tok_end(t));
	unistr_t str = _m_lex.token_string(t);
	std::string u8s;
	str.toUTF8String(u8s);
	switch (token_kind(t)) {
	case TokInt:
		node->_m_num.i_value = std::stoi(u8s);
		break;
	case TokFloat:
		node->_m_num.db_value= std::stod(u8s);
		break;
	}
	return node;
}

IRNode* Parser::parsing_id()
{
	token_t t = next_tok();
	return new IR_Id(t, tok_foffset(t), tok_end(t));
}

IRNode* Parser::parsing_literal()
{
	TokenKind tk = token_kind(token());
	switch (tk)
	{
	case TokId:
		return parsing_id();
	case TokInt:
	case TokFloat:
		return parsing_num();
	}
	printf("not support token kind for literal : %s\n", tk_to_str(tk).c_str());
	throw std::exception("not support kind literal");
}

IRNode* Parser::binary_parsing_proccess(
	std::function<bool(token_t)> fn_token_is_right_kind,
	std::function<IRNode*()> fn_xhand_side_parsing)
{
	IRNode* lhs = fn_xhand_side_parsing();
	token_t operator_tok;
	while (has_tok() && fn_token_is_right_kind(token()))
	{
		operator_tok = next_tok();
		if (!has_tok())
		{
			printf("doesn't has token when prccess right-hand side");
			throw std::exception("parsing binary node exception[ has no right-hand side expression ]");
		}
		IRNode* rhs = fn_xhand_side_parsing();
		lhs = new IR_BinOp(
			operator_tok, 
			tok_end(operator_tok),
			token_kind_to_tag(token_kind(operator_tok)),
			lhs, rhs);
	}
	return lhs;
}

bool token_is_mul(token_t t)
{
	switch (token_kind(t))
	{
	case TokDiv:
	case TokMul:
		return true;
	}
	return false;
}

IRNode* Parser::parsing_mul()
{
	return binary_parsing_proccess(&token_is_mul, [this]() { return parsing_literal(); });
}

bool token_is_add(token_t tok)
{
	switch (token_kind(tok))
	{
	case TokAdd:
	case TokSub:
		return true;
	}
	return false;
}

IRNode* Parser::parsing_add()
{
	return binary_parsing_proccess(&token_is_add, [this]() { return parsing_mul(); });
}

IRNode* TypeConvert::convert(IRNode* ir)
{
	if (nullptr == ir) return ir;
	return ir->accept(*this);
}

IRNode* TypeConvert::visit(IR_Num* num)
{
	if (num->is_float()) {
		push_t(CodeGen_D);
	}
	if (num->is_int()) {
		push_t(CodeGen_I);
	}
	return num;
}

IRNode* TypeConvert::visit(IR_BinOp* ir)
{
	CodeGenType rhs_t = pop_t();
	CodeGenType lhs_t = pop_t();
	if (lhs_t == rhs_t) {
		push_t(lhs_t);
		return ir;
	}
	else {
		CodeGenType t = better_type(lhs_t, rhs_t);
		push_t(t);
		if (t == lhs_t) {
			IRNode* irRHS = ir->rhs();
			IRNode* rhs = new IR_Cast(irRHS->end_loc(), irRHS, NodeDouble);
			return new IR_BinOp(ir->token(), ir->end_loc(), ir->tag(), ir->lhs(), rhs);
		}
		else {
			IRNode* irLHS = ir->lhs();
			IRNode* lhs = new IR_Cast(irLHS->end_loc(), irLHS, NodeDouble);
			return new IR_BinOp(ir->token(), ir->end_loc(), ir->tag(), lhs, ir->rhs());
		}
	}
}

IRNode* TypeConvert::visit(IR_Cast* id) 
{ 
	pop_t();
	if (id->cast_to() == NodeInt)
	{
		push_t(CodeGen_I);
	}
	if (id->cast_to() == NodeDouble)
	{
		push_t(CodeGen_D);
	}
	return id; 
}
void CodeGen::build(IRNode* ir)
{
	if (ir == nullptr) {
		printf("codegen error, ir is nullptr");
		return;
	}
}

IRNode* CodeGen::visit(IR_Num* num)
{
	CodeGenType t = gen_num(num);
	push_t(t);
	return num;
}

CodeGenType CodeGen::gen_num(IR_Num * ir) // -> CodeGenType
{
	IR_Num* num = ir;
#define InstrIConst(num) Instruction::iconst_ ## num
#define InstrDConst(num) Instruction::dconst_ ## num
	if (num->is_int())
	{
		FlInt iv = num->get_int();
		switch (iv)
		{
		case 0: add_instr(InstrIConst(0)); break;
		case 1: add_instr(InstrIConst(1)); break;
		case 2: add_instr(InstrIConst(2)); break;
		case 3: add_instr(InstrIConst(3)); break;
		case 4: add_instr(InstrIConst(4)); break;
		default:
			if (-32768 <= iv && iv <= 32767) {
				add_instr(Instruction::ipush);
				add_int16_to_code(iv);
			}
			else {
				uint8_t loc = _m_builder.store_const_int(iv);
				add_instr(Instruction::ldci);
				add_instr(loc);
			}
		}
		return CodeGen_I;
	}
	else if (num->is_float())
	{
		FlDouble dv = num->get_db();
		if (dv == 1.0 || dv == 2.0) {
			auto instr = dv == 1.0 ? InstrDConst(1) : InstrDConst(2);
			add_instr(instr);
		}
		else {
			uint8_t loc = _m_builder.store_const_double(dv);
			add_instr(Instruction::ldcd);
			add_instr(loc);
		}
		return CodeGen_D;
	}
	throw std::runtime_error("not support literal when gen code");
};

IRNode* CodeGen::visit(IR_BinOp* ir)
{
	// TODO:?
	return nullptr;
}

inline Instruction::Code tag_to_int_instr(IRNodeTag tag)
{
	switch (tag)
	{
		case IRTag_Add: return Instruction::iadd;
		case IRTag_Sub: return Instruction::isub;
		case IRTag_Mul: return Instruction::imul;
		case IRTag_Div: return Instruction::idiv;
	}
	throw std::exception("not support tag convert to instruction");
}

inline Instruction::Code tag_to_double_instr(IRNodeTag tag)
{
	switch (tag)
	{
		case IRTag_Add: return Instruction::dadd;
		case IRTag_Sub: return Instruction::dsub;
		case IRTag_Mul: return Instruction::dmul;
		case IRTag_Div: return Instruction::ddiv;
	}
	throw std::exception("not support tag convert to instruction");
}
CodeGenType CodeGen::gen_bin(IR_BinOp* ir)
{
	CodeGenType rhs_t = pop_t();
	CodeGenType lhs_t = top_t();
	if (lhs_t == rhs_t)
	{
		switch (lhs_t)
		{
			case CodeGen_I: 
				add_instr(tag_to_int_instr(ir->lhs()->tag()));
				break;
			case CodeGen_D: 
				add_instr(tag_to_double_instr(ir->lhs()->tag()));
				break;
			default:
				throw std::exception("not support other type when codegen");
		}
		pop_t();
		return lhs_t;
	}
	else 
	{
		// TODO:?
		CodeGenType btype = better_type(lhs_t, rhs_t);
		if (btype == rhs_t)
		{
		}
	}
}
