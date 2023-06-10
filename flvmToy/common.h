#pragma once
#include <stdint.h>
#include <string>
#include "unicode/unistr.h"

struct position_t {
	uint16_t mX;
	uint16_t mY;
	std::string to_string(const std::string spliter = ",") const {
		return std::to_string(mX) + spliter + std::to_string(mY);
	}

	position_t& next_line() { mY++;mX = 0;	return *this; }
	position_t& next_col()  { mX++;					return *this; }
};

typedef U_ICU_NAMESPACE::UnicodeString unistr_t;
