#ifndef __RENAME_H__
#define __RENAME_H__

#include "ast.h"
#include "environment.h"

class Renamer {
	public:
		Renamer(void);

		AST* rename(AST* ast);
	private:
		Environment<std::string, AST*> _values;
		Environment<std::string, AST*> _types;

		AST* rename_fun(AST* ast);
		AST* rename_let(AST* ast);
		AST* rename_variable(AST* ast);
		AST* rename_children(AST* ast);
};

#endif
