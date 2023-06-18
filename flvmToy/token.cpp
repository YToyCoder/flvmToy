#include "token.h"

std::string tk_to_str(token_t tok) {
	uint8_t* tk= (uint8_t*)(&tok);
	switch (*tk) {
	case TokId:  return "Id";
	case TokAdd: return "Add";
	case TokSub: return "Sub";
	case TokMul: return "Mul";
	case TokDiv: return "Div";
	case TokLParent: return "LParent";
	case TokRParent: return "RParent";
	case TokEol: return "Eol";
	case TokFloat:
	case TokInt:		return "Num";
	case TokAssign: return "ass";
	case TokLet:		return "let";
	case TokEq:			return "eq";
	case TokLt:			return "lt";
	case TokLe:			return "le";
	case TokGt:			return "gt";
	case TokGe:			return "ge";
	case TokIf: 		return "if";
	case TokElse: 	return "else";
	case TokLBrace: return "(";
	case TokRBrace: return ")";
	default:				return "?";
	}
}

token_t create_token(TokenKind _kind, uint32_t fileOffset, uint32_t token_offset){
	token_t tok = 0;
	uint8_t* byte_pointer = (uint8_t*)(&tok);
	*byte_pointer =(uint8_t) _kind;
	uint8_t* toffset= byte_pointer + 1;
	*toffset= token_offset;
	uint32_t* foffset = (uint32_t*)(byte_pointer + 2);
	*foffset = fileOffset;
	return tok;
}

bool token_is_kind(token_t tok,TokenKind _kind) {
	uint8_t* pkind= (uint8_t*)(&tok);
	return *pkind == _kind;
}

TokenKind token_kind(token_t tok) {
	return (TokenKind)*((char*)&tok);
}

uint32_t tok_foffset(token_t tok)
{
	uint8_t* byte_pointer = (uint8_t*)(&tok);
	uint32_t* foffset = (uint32_t*)(byte_pointer + 2);
	return *foffset ;
}

uint32_t tok_end(token_t tok)
{
	uint8_t* byte_pointer = (uint8_t*)(&tok);
	uint32_t* foffset = (uint32_t*)(byte_pointer + 2);
	return *foffset + tok_len(tok);
}

uint32_t tok_len(token_t tok)
{
	uint8_t* byte_pointer = (uint8_t*)(&tok);
	return *(byte_pointer + 1);
}

std::string token_to_str(token_t tok) {
	return 
		tk_to_str(tok) + "-" +
		std::to_string(tok_foffset(tok)) + "-" +
		std::to_string(tok_len(tok));
}


#ifdef TOK_TEST
#include <vector>
#include <stdio.h>

void tok_test1() {
}
#endif
