#pragma once
#include <string>
#include <stdint.h>

enum TokenKind : uint8_t
{
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
};

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

class Token {
public:
	std::pair<Position, Position> GetPosition() const 
	{ return std::make_pair(mStart, mEnd); }
	TokenKind kind() const { return mKind; }

	Token(const TokenKind _kind, const Position& _start, const Position& _end): 
		mKind(_kind), mStart(_start), mEnd(_end){}
private:
	TokenKind mKind;
	Position mStart;
	Position mEnd;
};
