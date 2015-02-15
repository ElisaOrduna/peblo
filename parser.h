#ifndef __PARSER_H__
#define __PARSER_H__

#include <vector>
#include "tokenizer.h"
#include "ast.h"

class Parser {
	public:
		Parser(Tokenizer& t);
		AST* parse_expression(void);
	private:
		Tokenizer& _tokenizer;

		typedef enum {
			OPERATOR_UNARY,
			OPERATOR_BINARY,
		} OperatorArity;

		std::vector<std::vector<TokenType > > _precedence_table;
		std::vector<OperatorArity> _arity_table;

		AST* parse_variable(void);
		AST* parse_constructor(void);
		AST* parse_atom(void);
		AST* parse_application(void);
		AST* parse_operators(unsigned int level);
		AST* parse_type(void);
		AST* parse_type_constructor(void);
		AST* parse_type_atom(bool allow_parameters);
		AST* parse_fun(void);
		AST* parse_let(void);
		AST* parse_type_declaration(void);
		AST* parse_constructor_declaration(void);
		AST* parse_variable_declaration(void);
		AST* parse_case(void);
		AST* parse_case_branch(void);
		void parse_parameter(AST* res);

		void assert_type(TokenType t);
		bool is_operator_at_level(TokenType t, unsigned int level);
};

#endif
