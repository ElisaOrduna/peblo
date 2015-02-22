#ifndef __INFER_H__
#define __INFER_H__

#include <map>

#include "ast.h"

typedef std::map<AST*, AST*> TypingContext;

class TypeInferrer {
	public:
		AST* infer(AST* ast);
	private:
		TypingContext _context;

		AST* infer_fun(AST* ast);
		AST* infer_variable(AST* var);
		void declare_variable_type(AST* var, AST* type);
};

#endif
