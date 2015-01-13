#include <cstdlib>
#include <cstdio>

#include "tokenizer.h"

using namespace std;

#define IS_DIGIT(C)		('0' <= (C) && (C) <= '9')
#define IS_UPPER(C)		('A' <= (C) && (C) <= 'Z')
#define IS_LOWER(C)		('a' <= (C) && (C) <= 'z')
#define IS_ALPHA(C)		(IS_UPPER(C) || IS_LOWER(C))
#define IS_IDENT(C)		((C) == '_' || IS_DIGIT(C) || IS_UPPER(C) || IS_LOWER(C))
#define IS_WHITESPACE(C)	((C) == ' ' || (C) == '\n' || (C) == '\t' || (C) == '\r')

Tokenizer::Tokenizer(istream& is) : _is(is) {
	if (_is.eof()) {
		_last_token.type = TOK_EOF;
		_last_token.value = "";
	} else {
		_last_token.type = TOK_BOF;
		_last_token.value = "";
		_char = _is.get();
		next();
	}
}

bool Tokenizer::eof(void) const {
	return _last_token.type == TOK_EOF;
}

Token Tokenizer::peek(void) const {
	return _last_token;
}

void Tokenizer::next(void) {
	while (IS_WHITESPACE(_char) && !_is.eof()) {
		_char = _is.get();
	}

	if (_char == EOF) {
		_last_token.type = TOK_EOF;
		_last_token.value = "";
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
		} else if (s == "in") {
			_last_token.type = TOK_IN;
		} else if (s == "case") {
			_last_token.type = TOK_CASE;
		} else if (s == "of") {
			_last_token.type = TOK_OF;
		} else if (s == "end") {
			_last_token.type = TOK_END;
		} else if (s == "var") {
			_last_token.type = TOK_VAR;
		} else if (s == "type") {
			_last_token.type = TOK_TYPE;
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
			_last_token.value = "<=";
		} else {
			_last_token.type = TOK_LT;
			_last_token.value = "<";
		}
	} else if (_char == '>'){
		_char = _is.get();
		if (_char == '=') {
			_char = _is.get();
			_last_token.type = TOK_GE;
			_last_token.value = ">=";
		} else {
			_last_token.type = TOK_GT;
			_last_token.value = ">";
		}
	} else if (_char == '!') {
		_char = _is.get();
		if (_char == '=') {
			_char = _is.get();
			_last_token.type = TOK_NE;
			_last_token.value = "!=";
		} else {
			_last_token.type = TOK_NOT;
			_last_token.value = "!";
		}
	} else if (_char == '=') {
		_char = _is.get();
		if (_char == '>') {
			_char = _is.get();
			_last_token.type = TOK_FAT_ARROW;
			_last_token.value = "=>";
		} else if (_char == '=') {
			_char = _is.get();
			_last_token.type = TOK_EQ;		
			_last_token.value = "==";
		} else {
			_last_token.type = TOK_DEF;		
			_last_token.value = "=";
		}
	} else if (_char == '(') {
		_char = _is.get();
		_last_token.type = TOK_LPAREN;
		_last_token.value = "(";
	} else if (_char == ')') {
		_char = _is.get();
		_last_token.type = TOK_RPAREN;
		_last_token.value = ")";
	} else if (_char == ':') {
		_char = _is.get();
		_last_token.type = TOK_COLON;
		_last_token.value = ":";
	} else if (_char == ';') {
		_char = _is.get();
		_last_token.type = TOK_SEMICOLON;
		_last_token.value = ";";
	} else if (_char == '+') {
		_char = _is.get();
		_last_token.type = TOK_PLUS;
		_last_token.value = "+";
	} else if (_char == '-') {
		_char = _is.get();
		if (_char == '>') {
			_char = _is.get();
			_last_token.type = TOK_ARROW;
			_last_token.value = "->";
		} else {
			_last_token.type = TOK_MINUS;
			_last_token.value = "-";
		}
	} else if (_char == '*') {
		_char = _is.get();
		_last_token.type = TOK_MUL;
		_last_token.value = "*";
	} else if (_char == '/') {
		_char = _is.get();
		_last_token.type = TOK_DIV;
		_last_token.value = "/";
	} else if (_char == '%') {
		_char = _is.get();
		_last_token.type = TOK_MOD;
		_last_token.value = "%";
	} else if (_char == '&') {
		_char = _is.get();
		if (_char == '&') {
			_char = _is.get();
			_last_token.type = TOK_AND;
			_last_token.value = "&&";
		} else {
			cerr << "Error: Simbolo no reconocido en la entrada: "
				<< "'" << (char)_char << "' (" << (int)_char << ")" << endl;
			exit(1);
		}
	} else if (_char == '|') {
		_char = _is.get();
		if (_char == '|') {
			_char = _is.get();
			_last_token.type = TOK_OR;
			_last_token.value = "||";
		} else {
			_last_token.type = TOK_PIPE;
			_last_token.value = "|";
		}
	} else {
		cerr << "Error: Simbolo no reconocido en la entrada: "
			<< "'" << (char)_char << "' (" << (int)_char << ")" << endl;
		exit(1);
	}
}

