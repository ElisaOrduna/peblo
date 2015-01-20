#include <cstdlib>
#include <iostream>

#include "rename.h"

using namespace std;

Renamer::Renamer() {}

AST* Renamer::rename_fun(AST* ast) {
	AST* param = ast->children[AST_FUN_PARAM];
	AST* symbol = make_variable_symbol(param->token.value);
	_values.push_def(param->token.value, symbol);
	ast->children[AST_FUN_PARAM] = symbol;

	if (ast->children[AST_FUN_PARAMTYPE] != NULL) {
		ast->children[AST_FUN_PARAMTYPE] =
			rename(ast->children[AST_FUN_PARAMTYPE]);
	}

	if (ast->children[AST_FUN_RETTYPE] != NULL) {
		ast->children[AST_FUN_RETTYPE] =
			rename(ast->children[AST_FUN_RETTYPE]);
	}

	ast->children[AST_FUN_BODY] = rename(ast->children[AST_FUN_BODY]);
	_values.pop_def(param->token.value);
	delete param;
	return ast;
}

AST* Renamer::rename_variable(AST* ast) {
	AST* res = _values.current_value(ast->token.value);
	delete ast;
	return res;
}

AST* Renamer::rename_children(AST* ast) {
	unsigned int i;
	for (i = 0; i < ast->children.size(); i++) {
		if (ast->children[i] != NULL) {
			ast->children[i] = rename(ast->children[i]);
		}
	}
	return ast;
}

AST* Renamer::rename(AST* ast) {
	switch (ast->kind) {
		case AST_FUN:
			return rename_fun(ast);
		case AST_VARIABLE:
			return rename_variable(ast);
		case AST_APP:
		case AST_CONSTANT:
		case AST_UNOP:
		case AST_BINOP:
		case AST_CONSTRUCTOR_DECLARATION:
		case AST_TYPE_ARROW:
			return rename_children(ast);


			/*
	// aparentemente no ligan nada
	AST_CONSTANT,
	AST_APP,
	AST_UNOP,
	AST_BINOP,
	AST_CONSTRUCTOR_DECLARATION,
	AST_TYPE_ARROW,

	AST_CONSTRUCTOR = 1,  // tratar como la variable
	AST_VARIABLE,         // tratar como la variable
	AST_CASE,             // liga las variables en los patrones
	AST_FUN,              // liga las variables ligadas
	AST_LET,              // liga muchas cosas:
	                      //   las variables declaradas,
	                      //   los tipos declarados, y los constructores
	AST_TYPE_DECLARATION,      // liga los parametros formales del tipo (ej. "type AB a = ...")
	AST_VARIABLE_DECLARATION,  // liga las variables de tipos polimorficos (ej. "var id : a -> a = ...")
	AST_TYPE_CONSTRUCTOR,	// tratar como la variable
	AST_TYPE_VAR,		// tratar como la variable

	let var x : a = z in x x
			*/

		default:
			cerr << "rename no implementado para " << ast->kind << endl;
			exit(1);
			return NULL;
	}
}
