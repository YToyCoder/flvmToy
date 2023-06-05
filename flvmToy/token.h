#pragma once
#include <string>
#include <stdint.h>

enum TokenKind : uint8_t
{
	// 
	TokId  = 0,
	// operand 
	TokAdd = 1, // +
	TokSub = 2, // -
	TokMul = 3, // *
	TokDiv = 4, // /
	TokLParent = 5, // (
	TokRParent = 6,  // )
	// control
	TokEol, // end of line => \n
	// not valid token
	TokNan,
};

enum TokendFlag : uint8_t
{
	TokFlagISTok = 0,
	TokFlagNOTTok = 1
};

std::string tk_to_str(uint8_t tk) {
	switch (tk) {
	case TokId:  return "Id";
	case TokAdd: return "Add";
	case TokSub: return "Sub";
	case TokMul: return "Mul";
	case TokDiv: return "Div";
	case TokLParent: return "LParent";
	case TokRParent: return "RParent";
	case TokEol: return "Eol";
	default:		 return "?";
	}
}

// 
struct Position {
	uint32_t mX;
	uint32_t mY;
	std::string to_string(const std::string spliter = ",") const {
		return std::to_string(mX) + spliter + std::to_string(mY);
	}

	Position& next_line() { mY++; return *this; }
	Position& next_col()  { mX++; return *this; }
};

// position: column row 16 * 2 = 32
// token --> unsigned long long
// <- 08 -> | <- 08 -> | <-- 16 --> | <-- 16 --> | <-- 16 -->
// |	  	  | tok-kind |     row    | start: col | end: row |
typedef uint64_t token_t;

#define InvalidTok (0x0100000000000000)
#define TokIsValid(t) (t == InvalidTok)

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
	*pend_col = row;
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
