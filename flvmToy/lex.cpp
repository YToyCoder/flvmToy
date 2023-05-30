#include "lex.h"
#include <stdio.h>


Lex::Lex(const std::string& rFilename): mFilename(rFilename), mState(LexState_Created) {
}

bool Lex::init() {
	mfstream.open(mFilename.c_str(),std::ios::out);
	mState = LexState_Init;
	return true;
}

bool Lex::check_state() const {
	if (mState == LexState_Init) { return true; }
	if (mState == LexState_Created) {
		printf("Lex haven't been initialized !!");
	}
	if (mState == LexState_Closed) {
		printf("Lex have been closed !!");
	}
	return false;
}

// todo
Token Lex::fetch_token() {
	return Token{ TokId, {0,0}, {0,0}};
}