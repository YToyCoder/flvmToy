#pragma once
#include <fstream>
#include <string>
#include "token.h"

template<typename _Ty, size_t size = 16 /* default size*/>
class FixedBuffer {
public:
	_Ty operator()() {
		if (m_read_cursor >= size) {
			throw std::exception(
				"buffer content is empty, write: " + 
				std::to_string(m_write_cursor) + 
				", read: " + std::to_string(m_read_cursor));
		}
		return mbuf[m_read_cursor++];
	}

	FixedBuffer& append(_Ty *pElems, size_t len) {
		if (m_write_cursor + len > size) {
			throw std::exception("append elems is too long");
		}
		for (size_t i = 0; i < len; i++) {
			mbuf[m_write_cursor++] = pElems[i];
		}
		return *this;
	}

	size_t rest_size() const { return m_write_cursor - m_read_cursor; }
private:
	_Ty mbuf[size];
	size_t m_read_cursor;
	size_t m_write_cursor;
};

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
