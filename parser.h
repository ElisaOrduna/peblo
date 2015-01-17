#ifndef __PARSER_H__
#define __PARSER_H__

#include <cstdlib>
#include <vector>
#include "tokenizer.h"

typedef enum {
	AST_CONSTRUCTOR = 1,
	AST_VARIABLE,
	AST_CONSTANT,
	AST_APP,
	AST_CASE,
	AST_FUN,
	AST_LET,
	AST_UNOP,
	AST_BINOP,
	AST_TYPE_DECLARATION,
	AST_VARIABLE_DECLARATION,
	AST_CONSTRUCTOR_DECLARATION,
	AST_PARAMETER,
	AST_TYPE_CONSTRUCTOR,
	AST_TYPE_VAR,
	AST_TYPE_ARROW,
} ASTKind;

#define AST_APP_FUNCTION	0
#define AST_APP_ARGUMENT	1

#define AST_FUN_PARAM		0
#define AST_FUN_RETTYPE		1
#define AST_FUN_BODY		2

struct AST {
	ASTKind kind;
	Token token;
	std::vector<AST*> children;

	void show(std::ostream& os, unsigned int level = 0) const;
};

typedef enum {
	OPERATOR_UNARY,
	OPERATOR_BINARY,
} OperatorArity;

class Parser {
	public:
		Parser(Tokenizer& t);
		
		AST* parse_atom(void);
		AST* parse_expression(void);
		AST* parse_application(void);
		AST* parse_operators(unsigned int level);
		AST* parse_type(void);
		AST* parse_type_atom(bool allow_parameters);
		AST* parse_fun(void);
		AST* parse_let(void);
		AST* parse_type_declaration(void);
		AST* parse_constructor_declaration(void);
		AST* parse_variable_declaration(void);
		AST* parse_case(void);
		AST* parse_parameter(void);
	private:
		Tokenizer& _tokenizer;
		std::vector<std::vector<TokenType > > _precedence_table;
		std::vector<OperatorArity> _arity_table;

		void assert_type(TokenType t);
		bool is_operator_at_level(TokenType t, unsigned int level);
};

#endif
