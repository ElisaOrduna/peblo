#ifndef __RENAME_H__
#define __RENAME_H__

#include "ast.h"
#include "environment.h"

typedef Environment<std::string, AST*> SymbolEnv;

class Renamer {
	public:
		Renamer(void);

		AST* rename(AST* ast);
	private:
		SymbolEnv _values;
		SymbolEnv _types;

		AST* rename_fun(AST* ast);
		AST* rename_let(AST* ast);
		AST* rename_case_branch(AST* ast);
		AST* rename_variable(SymbolEnv& env, AST* ast);
		AST* rename_children(AST* ast);
		AST* rename_variable_declaration(AST* ast);
		AST* rename_type_declaration(AST* ast);
};

#endif
