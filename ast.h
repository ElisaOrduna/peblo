#ifndef __AST_H__
#define __AST_H__

#include <vector>
#include "tokenizer.h"

typedef enum {
	// Nodos que construye el parser
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
	AST_TYPE_CONSTRUCTOR,
	AST_TYPE_VAR,
	AST_TYPE_ARROW,

	// Nodos usados en la etapa de renombre
	AST_CONSTRUCTOR_SYMBOL,
	AST_VARIABLE_SYMBOL,
} ASTKind;

#define AST_APP_FUNCTION	0
#define AST_APP_ARGUMENT	1

#define AST_FUN_PARAM		0
#define AST_FUN_PARAMTYPE	1
#define AST_FUN_RETTYPE		2
#define AST_FUN_BODY		3

struct AST {
	ASTKind kind;
	Token token;
	std::vector<AST*> children;

	void show(std::ostream& os, unsigned int level = 0) const;
};

AST* make_variable_symbol(const std::string& name);

#endif
