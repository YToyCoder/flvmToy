#include "token.h"

std::string tk_to_str(token_t tok) {
	uint8_t* tk= (uint8_t*)(&tok) + 1;
	switch (*tk) {
	case TokId:  return "Id";
	case TokAdd: return "Add";
	case TokSub: return "Sub";
	case TokMul: return "Mul";
	case TokDiv: return "Div";
	case TokLParent: return "LParent";
	case TokRParent: return "RParent";
	case TokEol: return "Eol";
	case TokNum: return "Num";
	default:		 return "?";
	}
}

token_t create_token(TokenKind _kind, uint16_t row, uint16_t start_col, uint16_t end_col) {
	token_t tok = 0x0;
	uint8_t* byte_pointer = (uint8_t*)(&tok);
	*byte_pointer = TokFlagISTok;
	uint8_t* kind_p = byte_pointer + 1;
	*kind_p = _kind;
	uint16_t* prow = (uint16_t*)(byte_pointer + 2);
	*prow = row;
	uint16_t* pstart_col = (uint16_t*)(byte_pointer + 4);
	*pstart_col = start_col;
	uint16_t* pend_col = (uint16_t*)(byte_pointer + 6);
	*pend_col = end_col;
	return tok;
}

bool token_is_kind(token_t tok,TokenKind _kind) {
	uint8_t* pkind= (uint8_t*)(&tok) + 1;
	return *pkind == _kind;
}

uint16_t token_row(token_t tok) {
	uint16_t* pos= (uint16_t*)(&tok) + 1;
	return *pos;
}

uint16_t token_scol(token_t tok) {
	uint16_t* pos= (uint16_t*)(&tok) + 2;
	return *pos;
}

uint16_t token_ecol(token_t tok) {
	uint16_t* pos= (uint16_t*)(&tok) + 3;
	return *pos;
}

std::string token_to_str(token_t tok) {
	return tk_to_str(tok) + "-" +
				 std::to_string(token_row(tok)) + "-" + 
				 std::to_string(token_scol(tok)) + "-" + 
				 std::to_string(token_ecol(tok));
}


#ifdef TOK_TEST
#include <vector>
#include <stdio.h>

void tok_test1() {
	std::vector<token_t> vec;

	vec.push_back(create_token(TokId, 0, 1, 4));
	vec.push_back(create_token(TokId, 0, 1, 4));
	vec.push_back(create_token(TokId, 0, 1, 4));
	vec.push_back(create_token(TokId, 0, 1, 4));
	vec.push_back(create_token(TokId, 0, 1, 4));
	for (auto& t : vec) {
		printf("%s\n", token_to_str(t).c_str());
	}
}
#endif
