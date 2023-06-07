#pragma once
#include "lex.h"
#include "ir.h"

class Parser
{
public:
	IRNode* parse();
protected:
	IRNode* parsing_num();
	IRNode* parsing_id();
private:
	Lex _m_lex;
};

