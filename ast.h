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
	AST_CASE_BRANCH,
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
	AST_TYPE_APP,

	// Nodos usados en la etapa de renombre
	AST_CONSTRUCTOR_SYMBOL,
	AST_VARIABLE_SYMBOL,

	// Usados en la etapa de tipado
	AST_TYPE_METAVAR,

} ASTKind;

#define AST_APP_FUNCTION	0
#define AST_APP_ARGUMENT	1

#define AST_FUN_PARAM		0
#define AST_FUN_PARAMTYPE	1
#define AST_FUN_RETTYPE		2
#define AST_FUN_BODY		3

#define AST_VARDECL_VAR		0
#define AST_VARDECL_VARTYPE	1
#define AST_VARDECL_BODY	2

#define AST_TYPEDECL_TYPE		0
#define AST_TYPEDECL_FIRST_CONSTRUCTOR	1

#define AST_CASE_BRANCH_PATTERN	0
#define AST_CASE_BRANCH_BODY	1

#define AST_TYPE_ARROW_DOMAIN	0
#define AST_TYPE_ARROW_CODOMAIN	1

struct AST {
	ASTKind kind;
	Token token;
	std::vector<AST*> children;

	void show(std::ostream& os, unsigned int level = 0) const;

	friend std::ostream& operator<<(std::ostream& os, AST* ast);
};

AST* make_symbol(const std::string& name, ASTKind symbol_kind);

#endif
