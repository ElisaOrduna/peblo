#include <cstdlib>
#include <cassert>

#include "infer.h"
#include "unify.h"

using namespace std;

#define make_fresh_metavar()	make_metavar("t")

void TypeInferrer::declare_variable_type(AST* var, AST* type) {
	assert(_context.find(var) == _context.end());
	_context[var] = type;
}

AST* TypeInferrer::infer_fun(AST* ast) {
	if (ast->children[AST_FUN_PARAMTYPE] == NULL) {
		ast->children[AST_FUN_PARAMTYPE] = make_fresh_metavar();
	}

	if (ast->children[AST_FUN_RETTYPE] == NULL) {
		ast->children[AST_FUN_RETTYPE] = make_fresh_metavar();
	}

	AST* param_type = ast->children[AST_FUN_PARAMTYPE];
	AST* ret_type = ast->children[AST_FUN_RETTYPE];

	declare_variable_type(ast->children[AST_FUN_PARAM], param_type);

	AST* body_type = infer(ast->children[AST_FUN_BODY]);

	if (!unify(ret_type, body_type)) {
		cerr << "El cuerpo de la funcion no es del tipo esperado" << endl;
		cerr << "Tipo del cuerpo: " << body_type << endl;
		cerr << "Tipo esperado:   " << ret_type << endl;
		exit(1);
	}

	return make_type_arrow(param_type, ret_type);
}

AST* TypeInferrer::infer_variable(AST* var) {
	assert(_context.find(var) != _context.end());
	return _context.at(var);
}

AST* TypeInferrer::infer(AST* ast) {
	switch (ast->kind) {
		case AST_VARIABLE_SYMBOL:
			return infer_variable(ast);
		case AST_FUN:
			return infer_fun(ast);	
		case AST_VARIABLE:
		case AST_CONSTRUCTOR:
			cerr << "No debe haber nodos de tipo: " << ast->kind << endl;
			cerr << "Probablemente falte la etapa de renombre" << endl;
			exit(1);
		default:
			cerr << "Inferencia no implementada para: " << ast->kind << endl;
			exit(1);
	} 	
}
