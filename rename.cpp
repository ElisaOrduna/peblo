#include <cstdlib>
#include <iostream>

#include "rename.h"

using namespace std;

Renamer::Renamer() {}

void bind_variable(Environment<string, AST*>& env, AST** old_ast, ASTKind symbol_kind) {
	AST* symbol = make_symbol((*old_ast)->token.value, symbol_kind);

	if (env.defined_in_current_scope((*old_ast)->token.value)) {
		cerr << "Esta variable ya fue definida en el scope: ";
		cerr << (*old_ast)->token.value << endl;
		exit(1);
	}

	env.push_def((*old_ast)->token.value, symbol);
	delete *old_ast;
	*old_ast = symbol;
}

AST* Renamer::rename_fun(AST* ast) {
	_values.begin_scope();
	bind_variable(_values, &ast->children[AST_FUN_PARAM], AST_VARIABLE_SYMBOL);

	ast = rename_children(ast);

	_values.end_scope();
	return ast;
}

AST* Renamer::rename_variable(AST* ast) {
	if (!_values.defined(ast->token.value)) {
		cerr << "variable no declarada: " << ast->token.value << endl;
		exit(1);
	}

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

AST* Renamer::rename_let(AST* ast) {
	_values.begin_scope();
	_types.begin_scope();

	unsigned int i;
	for (i = 0; i < ast->children.size() - 1; i++) {
		AST* decl = ast->children[i];
		if (decl->kind == AST_TYPE_DECLARATION) {
			AST** type_constructor = &decl->children[AST_TYPEDECL_TYPE]->children[0];
			bind_variable(_types, type_constructor, AST_CONSTRUCTOR_SYMBOL);

			// TODO: constructores

		} else if (decl->kind == AST_VARIABLE_DECLARATION) {
			bind_variable(_values, &decl->children[AST_VARDECL_VAR], AST_VARIABLE_SYMBOL);
		} else {
			cerr << "No es una declaracion conocida: " << decl->kind << endl;
			exit(1);
		}
	}

	ast = rename_children(ast);

	_values.end_scope();
	_types.end_scope();
	return ast;
}

AST* Renamer::rename(AST* ast) {
	switch (ast->kind) {
		case AST_FUN:
			return rename_fun(ast);
		case AST_LET:
			return rename_let(ast);
		case AST_VARIABLE:
		case AST_CONSTRUCTOR:
			return rename_variable(ast);
		case AST_VARIABLE_DECLARATION:
			return rename_children(ast); //TODO 
		case AST_APP:
		case AST_CONSTANT:
		case AST_UNOP:
		case AST_BINOP:
		case AST_CONSTRUCTOR_DECLARATION:
		case AST_TYPE_ARROW:
		case AST_TYPE_APP:
			return rename_children(ast);
		// Si encuentro un simbolo, es porque ya lo renombre
		case AST_VARIABLE_SYMBOL:
		case AST_CONSTRUCTOR_SYMBOL:
			return ast;

			/*
	// aparentemente no ligan nada
	AST_CONSTANT,
	AST_APP,
	AST_UNOP,
	AST_BINOP,
	AST_CONSTRUCTOR_DECLARATION,
	AST_TYPE_ARROW,
	AST_TYPE_APP,

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
