#include <cstdlib>
#include <iostream>

#include "rename.h"

using namespace std;

Renamer::Renamer() {}

AST* bind_variable(SymbolEnv& env, const string& name, ASTKind symbol_kind) {
	AST* symbol = make_symbol(name, symbol_kind);

	if (env.defined_in_current_scope(name)) {
		cerr << "Esta variable ya fue definida en el scope: ";
		cerr << name << endl;
		exit(1);
	}

	env.push_def(name, symbol);
	return symbol;
}

void bind_variable_and_replace(SymbolEnv& env, AST** old_ast, ASTKind symbol_kind) {
	AST* symbol = bind_variable(env, (*old_ast)->token.value, symbol_kind);
	delete *old_ast;
	*old_ast = symbol;
}

AST* Renamer::rename_fun(AST* ast) {
	_values.begin_scope();
	bind_variable_and_replace(_values, &ast->children[AST_FUN_PARAM], AST_VARIABLE_SYMBOL);

	ast = rename_children(ast);

	_values.end_scope();
	return ast;
}

AST* Renamer::rename_variable(SymbolEnv& env, AST* ast) {
	if (!env.defined(ast->token.value)) {
		cerr << "variable no declarada: " << ast->token.value << endl;
		exit(1);
	}

	AST* res = env.current_value(ast->token.value);
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

void bind_constructors(SymbolEnv& values, vector<AST*>& children) {
	unsigned int i;
	for (i = AST_TYPEDECL_FIRST_CONSTRUCTOR; i < children.size(); i++) {
		assert(children[i]->kind == AST_CONSTRUCTOR_DECLARATION);
		bind_variable_and_replace(values, &children[i]->children[0], AST_CONSTRUCTOR_SYMBOL);
	}
}

AST* Renamer::rename_let(AST* ast) {
	_values.begin_scope();
	_types.begin_scope();

	unsigned int i;
	for (i = 0; i < ast->children.size() - 1; i++) {
		AST* decl = ast->children[i];
		if (decl->kind == AST_TYPE_DECLARATION) {
			AST** type_constructor = &decl->children[AST_TYPEDECL_TYPE]->children[0];
			bind_variable_and_replace(_types, type_constructor, AST_CONSTRUCTOR_SYMBOL);
			bind_constructors(_values, decl->children);
		} else if (decl->kind == AST_VARIABLE_DECLARATION) {
			bind_variable_and_replace(_values, &decl->children[AST_VARDECL_VAR], AST_VARIABLE_SYMBOL);
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

void free_pattern_variables(AST* ast, set<string>& freevars) {
	if (ast->kind == AST_VARIABLE) {
		freevars.insert(ast->token.value);
	} else if (ast->kind == AST_CONSTRUCTOR) {
		// no tiene variables libres	
	} else if (ast->kind == AST_APP) {
		free_pattern_variables(ast->children[AST_APP_FUNCTION], freevars);
		free_pattern_variables(ast->children[AST_APP_ARGUMENT], freevars);
	} else {
		cerr << "No es un patron: " << ast->kind << endl;
		exit(1);
	}
}

AST* Renamer::rename_case_branch(AST* ast) {
	set<string> freevars;
	free_pattern_variables(ast->children[AST_CASE_BRANCH_PATTERN], freevars);	

	_values.begin_scope();
	for (set<string>::iterator v = freevars.begin(); v != freevars.end(); ++v) {
		bind_variable(_values, *v, AST_VARIABLE_SYMBOL);
	}
	ast = rename_children(ast);
	_values.end_scope();	

	return ast;
}

void free_type_variables(AST* ast, set<string>& freevars) {
	if (ast->kind == AST_TYPE_VAR) {
		freevars.insert(ast->token.value);
	} else if (ast->kind == AST_TYPE_CONSTRUCTOR) {
		// no tiene variables libres
	} else if (ast->kind == AST_TYPE_APP) {
		unsigned int i;
		for (i = 0; i < ast->children.size(); i++) {
			free_type_variables(ast->children[i], freevars);
		}
	} else if (ast->kind == AST_TYPE_ARROW) {
		free_type_variables(ast->children[AST_TYPE_ARROW_DOMAIN], freevars);
		free_type_variables(ast->children[AST_TYPE_ARROW_CODOMAIN], freevars);
	} else {
		cerr << "No es un tipo: " << ast->kind << endl;
		exit(1);
	}
}

AST* Renamer::rename_variable_declaration(AST* ast) {
	assert(ast->kind == AST_VARIABLE_DECLARATION);
	
	if (ast->children[AST_VARDECL_VARTYPE] != NULL) {
		set<string> freevars;
		free_type_variables(ast->children[AST_VARDECL_VARTYPE], freevars);

		_types.begin_scope();
		for (set<string>::iterator v = freevars.begin(); v != freevars.end(); ++v) {
			bind_variable(_types, *v, AST_VARIABLE_SYMBOL);
		}
		ast->children[AST_VARDECL_VARTYPE] = rename(ast->children[AST_VARDECL_VARTYPE]);
		_types.end_scope();
	}

	ast->children[AST_VARDECL_BODY] = rename(ast->children[AST_VARDECL_BODY]);

	return ast;	
}

AST* Renamer::rename_type_declaration(AST* ast) {
	assert(ast->kind == AST_TYPE_DECLARATION);
	
	_types.begin_scope();
	AST* declared_type = ast->children[AST_TYPEDECL_TYPE];
	unsigned int i;
	for (i = 1; i < declared_type->children.size(); i++) {
		bind_variable_and_replace(_types, &declared_type->children[i], AST_VARIABLE_SYMBOL);
	}
	ast = rename_children(ast);
	_types.end_scope();

	return ast;
}

AST* Renamer::rename(AST* ast) {
	switch (ast->kind) {
		case AST_FUN:
			return rename_fun(ast);
		case AST_LET:
			return rename_let(ast);
		case AST_CASE_BRANCH:
			return rename_case_branch(ast);
		case AST_VARIABLE:
		case AST_CONSTRUCTOR:
			return rename_variable(_values, ast);
		case AST_TYPE_VAR:
		case AST_TYPE_CONSTRUCTOR:
			return rename_variable(_types, ast);
		case AST_VARIABLE_DECLARATION:
			return rename_variable_declaration(ast);
		case AST_TYPE_DECLARATION:
			return rename_type_declaration(ast);
		case AST_CASE:
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
