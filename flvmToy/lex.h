#pragma once
#include <fstream>
#include <string>
#include "token.h"

class Lex {
public:
	Lex(const std::string& rFilename);
	bool init();
protected:
	enum LexState {
		LexState_Created = 0,
		LexState_Init ,
		LexState_Closed
	};
	// checking state
	bool check_state() const ;
	Token fetch_token();
private:
	std::string mFilename;
	std::fstream mfstream;
	LexState mState;
};
