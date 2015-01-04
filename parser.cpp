#include <iostream>
#include <cstdlib>
#include <cstdio>

using namespace std;

typedef enum {
	TOK_UPPERID,
	TOK_LOWERID,
	TOK_NUMBER,
	/* Palabras reservadas */
	TOK_FUN,
	TOK_LET,
	/* Operadores */
	TOK_DEF,	// =
	TOK_EQ,		// ==
	TOK_NE,		// !=
	TOK_LE,		// <=
	TOK_LT,		// <
	TOK_GE,		// >=
	TOK_GT,		// >
	TOK_NOT,	// !
	TOK_FAT_ARROW,	// =>
} TokenType;

struct Token {
	TokenType type;
	string value;
};

class Tokenizer {
	public:
		Tokenizer(istream& is);
		bool eof(void) const;
		Token peek(void) const;
		void next(void);
	private:
		istream& _is;
		Token _last_token;
		int _char;
		bool _eof;
};

#define IS_DIGIT(C)		('0' <= (C) && (C) <= '9')
#define IS_UPPER(C)		('A' <= (C) && (C) <= 'Z')
#define IS_LOWER(C)		('a' <= (C) && (C) <= 'z')
#define IS_ALPHA(C)		(IS_UPPER(C) || IS_LOWER(C))
#define IS_IDENT(C)		((C) == '_' || IS_DIGIT(C) || IS_UPPER(C) || IS_LOWER(C))
#define IS_WHITESPACE(C)	((C) == ' ' || (C) == '\n' || (C) == '\t' || (C) == '\r')

Tokenizer::Tokenizer(istream& is) : _is(is) {
	if (_is.eof()) {
		_eof = true;
	} else {
		_eof = false;
		_char = _is.get();
		next();
	}
}

bool Tokenizer::eof(void) const {
	return _is.eof();
}

Token Tokenizer::peek(void) const {
	return _last_token;
}

void Tokenizer::next(void) {
	while (IS_WHITESPACE(_char) && !_is.eof()) {
		_char = _is.get();
	}

	if (_char == EOF) {
		_eof = true;
		return;
	}
	
	if (IS_IDENT(_char)) {
		bool all_digit = true;
		string s = "";
		while (IS_IDENT(_char) && !_is.eof()) {
			all_digit = all_digit && IS_DIGIT(_char);
			s.push_back(_char);
			_char = _is.get();
		}

		_last_token.value = s;
		if (all_digit) {
			_last_token.type = TOK_NUMBER;
		} else if (s == "fun") {
			_last_token.type = TOK_FUN;
		} else if (s == "let") {
			_last_token.type = TOK_LET;
		} else {
			// No es una palabra reservada
			if (IS_UPPER(s[0])) {
				_last_token.type = TOK_UPPERID;
			} else {
				_last_token.type = TOK_LOWERID;
			}
		}
	} else if (_char == '<') {
		_char = _is.get();
		if (_char == '=') {
			_char = _is.get();
			_last_token.type = TOK_LE;
		} else {
			_last_token.type = TOK_LT;
		}
	} else if (_char == '>'){
		_char = _is.get();
		if (_char == '=') {
			_char = _is.get();
			_last_token.type = TOK_GE;
		} else {
			_last_token.type = TOK_GT;
		}
	} else if (_char == '!') {
		_char = _is.get();
		if (_char == '=') {
			_last_token.type = TOK_NE;
		} else {
			_last_token.type = TOK_NOT;
		}
	} else if (_char == '=') {
		_char = _is.get();
		if (_char == '>') {
			_last_token.type = TOK_FAT_ARROW;
		} else if (_char == '=') {
			_last_token.type = TOK_EQ;		
		} else {
			_last_token.type = TOK_DEF;		
		}
	} else {
		cerr << "Error: Simbolo no reconocido en la entrada: "
			<< "'" << (char)_char << "' (" << (int)_char << ")" << endl;
		exit(1);
	}
}

#include <fstream>
int main() {
	ifstream input("test.txt");
	Tokenizer tok(input);
	while (!tok.eof()) {
		Token t = tok.peek();
		cout << "lei <type=" << t.type << ", value=" << t.value << ">" << endl;
		tok.next();
	}
	return 0;
}
