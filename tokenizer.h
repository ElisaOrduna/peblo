#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <iostream>

typedef enum {
	TOK_UPPERID,
	TOK_LOWERID,
	TOK_NUMBER,
	/* Palabras reservadas */
	TOK_FUN,
	TOK_LET,
	TOK_IN,
	TOK_VAR,
	TOK_TYPE,
	TOK_CASE,
	TOK_OF,
	TOK_END,
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
	TOK_LPAREN,	// (
	TOK_RPAREN,	// )
	TOK_COLON,	// :
	TOK_SEMICOLON,	// ;
	TOK_PLUS,	// +
	TOK_MINUS,	// -
	TOK_MUL,	// *
	TOK_DIV,	// /
	TOK_MOD,	// %
	TOK_AND,	// &&
	TOK_OR,		// ||
	TOK_PIPE,	// |
	TOK_ARROW,	// ->
} TokenType;

struct Token {
	TokenType type;
	std::string value;
};

class Tokenizer {
	public:
		Tokenizer(std::istream& is);
		bool eof(void) const;
		Token peek(void) const;
		void next(void);
	private:
		std::istream& _is;
		Token _last_token;
		int _char;
		bool _eof;
};

#endif
