#include <cstdlib>
#include "parser.h"

using namespace std;

void AST::show(ostream& os, unsigned int level) const {
	unsigned int i;
	for (i = 0; i < level; i++) {
		os << "  ";
	}
	os << "(" << kind << " " << token.type << " " << token.value;
	for (i = 0; i < children.size(); i++) {
		os << endl;
		if (children[i] != NULL) { 
			children[i]->show(os, level + 1);
		} else {
			unsigned int j;
			for (j = 0; j < level + 1; j++) {
				os << "  ";
			}
			os << "NULL";
		}
	}
	os << ")"; 
}

Parser::Parser(Tokenizer& t) : _tokenizer(t) {
	_precedence_table = vector<vector<TokenType> >(9);
	_arity_table = vector<OperatorArity>(9, OPERATOR_BINARY);
	
	// Operadores logicos
	_precedence_table[0].push_back(TOK_OR);
	_precedence_table[1].push_back(TOK_AND);
	_precedence_table[2].push_back(TOK_NOT); // Unario
	_arity_table[2] = OPERATOR_UNARY;

	// Operadores relacionales
	_precedence_table[3].push_back(TOK_EQ);
	_precedence_table[3].push_back(TOK_NE);
	_precedence_table[3].push_back(TOK_GE);
	_precedence_table[3].push_back(TOK_GT);
	_precedence_table[3].push_back(TOK_LE);
	_precedence_table[3].push_back(TOK_LT);

	// Operadores aritmeticos
	_precedence_table[4].push_back(TOK_PLUS);
	_precedence_table[5].push_back(TOK_MINUS);
	_precedence_table[6].push_back(TOK_MUL);
	_precedence_table[7].push_back(TOK_DIV);
	_precedence_table[7].push_back(TOK_MOD);
	_precedence_table[8].push_back(TOK_MINUS); // Unario
	_arity_table[8] = OPERATOR_UNARY;
}

AST* Parser::parse_atom(void) {
	Token t = _tokenizer.peek();

	if (t.type == TOK_UPPERID) {
		AST* res = new AST();
		res->kind = AST_CONSTRUCTOR;
		res->token = t;
		_tokenizer.next();
		return res;
	} else if (t.type == TOK_LOWERID) {
		AST* res = new AST();
		res->kind = AST_VARIABLE;
		res->token = t;
		_tokenizer.next();
		return res;
	} else if (t.type == TOK_NUMBER) {
		AST* res = new AST();
		res->kind = AST_CONSTANT;
		res->token = t;
		_tokenizer.next();
		return res;
	} else if (t.type == TOK_LPAREN) {
		AST *res;
		_tokenizer.next();
		res = parse_expression();
		if (_tokenizer.peek().type != TOK_RPAREN) {
			cerr << "Error de parsing: se esperaba un ')'." << endl;
			exit(1);
		}
		_tokenizer.next();
		return res;
	} else {
		cerr << "Error de parsing" << endl;
		cerr << "Se encontro " << t.type << " [ " << t.value << " ]" << endl;
		exit(1);
	}
}

bool is_application_terminator(TokenType t) {
	return		t == TOK_EOF
		||	t == TOK_RPAREN
		||	t == TOK_OF
		||	t == TOK_FAT_ARROW
		||	t == TOK_PIPE
		||	t == TOK_END
		||	t == TOK_VAR
		||	t == TOK_TYPE
		||	t == TOK_IN
		// Operadores
		||	t == TOK_OR
		||	t == TOK_AND
		||	t == TOK_NOT
		||	t == TOK_EQ
		||	t == TOK_NE
		||	t == TOK_GE
		||	t == TOK_GT
		||	t == TOK_LE
		||	t == TOK_LT
		||	t == TOK_PLUS
		||	t == TOK_MINUS
		||	t == TOK_MUL
		||	t == TOK_DIV
		||	t == TOK_MOD;
}

bool is_parameter_terminator(TokenType t) {
	return		t == TOK_FAT_ARROW
		||	t == TOK_COLON;
}

bool is_type_terminator(TokenType t) {
	return		t == TOK_RPAREN
		||	t == TOK_ARROW
		||	t == TOK_FAT_ARROW
		||	t == TOK_DEF;
}

void Parser::assert_type(TokenType t) {
	if (_tokenizer.peek().type != t) {
		cerr << "Error de sintaxis" << endl;
		cerr << "Se esperaba token " << t << endl;
		cerr << "Se recibio token " << _tokenizer.peek().type << endl;
		exit(1);
	}
}

AST* Parser::parse_type(void) {
	AST* res = parse_type_atom(true);
	if (_tokenizer.peek().type == TOK_ARROW) {
		_tokenizer.next();
		AST* aux = new AST();
		aux->kind = AST_TYPE_ARROW;
		aux->children.push_back(res);
		aux->children.push_back(parse_type());
		res = aux;
	}
	return res;
}

AST* Parser::parse_type_atom(bool allow_parameters) {
	if (_tokenizer.peek().type == TOK_LPAREN) {
		_tokenizer.next();
		AST* res = parse_type();
		assert_type(TOK_RPAREN);
		_tokenizer.next();
		return res;
	} else if (_tokenizer.peek().type == TOK_UPPERID) {
		AST* res = new AST();
		res->kind = AST_TYPE_CONSTRUCTOR;
		res->token = _tokenizer.peek();
		_tokenizer.next();
		if (allow_parameters) {
			while (!is_type_terminator(_tokenizer.peek().type)) {
				res->children.push_back(parse_type_atom(false));
			}
		}
		return res;
	} else {
		assert_type(TOK_LOWERID);
		AST* res = new AST();
		res->kind = AST_TYPE_VAR;
		res->token = _tokenizer.peek();

		_tokenizer.next();
		return res;
	}
}

AST* Parser::parse_parameter(void) {
	AST* res = new AST();
	res->kind = AST_PARAMETER;
	if (_tokenizer.peek().type == TOK_LPAREN) {
		_tokenizer.next();

		assert_type(TOK_LOWERID);
		res->token = _tokenizer.peek(); // Nombre parametro
		_tokenizer.next();
		
		assert_type(TOK_COLON);
		_tokenizer.next();

		res->children.push_back(parse_type()); // Tipo parametro

		assert_type(TOK_RPAREN);
		_tokenizer.next();
	} else {
		assert_type(TOK_LOWERID);
		res->token = _tokenizer.peek(); // Nombre parametro
		_tokenizer.next();

		res->children.push_back(NULL); // Tipo parametro
	}
	return res;
}

AST* Parser::parse_fun(void) {
	assert_type(TOK_FUN);
	_tokenizer.next();

	AST* res = new AST();
	res->kind = AST_FUN;
	res->children.push_back(parse_parameter());
	res->children.push_back(NULL); // Tipo de retorno
	res->children.push_back(NULL); // Cuerpo de la funcion
	AST* last = res;
	while (!is_parameter_terminator(_tokenizer.peek().type)) {
		AST* aux = new AST();
		aux->kind = AST_FUN;
		aux->children.push_back(parse_parameter());
		aux->children.push_back(NULL); // Tipo de retorno
		aux->children.push_back(NULL); // Cuerpo de la funcion
		
		last->children[AST_FUN_BODY] = aux;
		last = aux;
	}
	
	if (_tokenizer.peek().type == TOK_COLON) {
		_tokenizer.next();
		last->children[AST_FUN_RETTYPE] = parse_type();
	}
	
	assert_type(TOK_FAT_ARROW);
	_tokenizer.next();

	last->children[AST_FUN_BODY] = parse_expression();
	return res;
}

bool is_pattern(AST* pattern) {
	if (pattern->kind == AST_VARIABLE) {
		return true;
	}
	while (pattern->kind == AST_APP) {
		if (!is_pattern(pattern->children[AST_APP_ARGUMENT])) {
			return false;
		}
		pattern = pattern->children[AST_APP_FUNCTION];
	}
	return pattern->kind == AST_CONSTRUCTOR;
}

bool is_type_constructor_declaration(AST* type_constructor) {
	if (type_constructor->kind != AST_TYPE_CONSTRUCTOR) {
		return false;
	}

	unsigned int i;
	for (i = 0; i < type_constructor->children.size(); i++) {
		if (type_constructor->children[i]->kind != AST_TYPE_VAR) {
			return false;
		}
	}
	return true;
}

bool is_constructor_terminator(TokenType t) {
	return		t == TOK_TYPE
		||	t == TOK_VAR
		||	t == TOK_IN
		||	t == TOK_PIPE;
}

AST* Parser::parse_constructor_declaration(void) {
	AST* res = new AST();
	res->kind = AST_CONSTRUCTOR_DECLARATION;

	assert_type(TOK_UPPERID);
	res->token = _tokenizer.peek();
	_tokenizer.next();

	while (!is_constructor_terminator(_tokenizer.peek().type)) {
		res->children.push_back(parse_type_atom(false));		
	}
	return res;
}

AST* Parser::parse_type_declaration(void) {
	assert_type(TOK_TYPE);
	_tokenizer.next();

	AST* res = new AST();
	res->kind = AST_TYPE_DECLARATION;
	
	AST* type_constructor = parse_type();
	if (!is_type_constructor_declaration(type_constructor)) {
		cerr << "No es una declaracion de constructor de tipos" << endl;
		type_constructor->show(cerr);
		cerr << endl;
		exit(1);
	}
	res->children.push_back(type_constructor);

	assert_type(TOK_DEF);
	_tokenizer.next();

	res->children.push_back(parse_constructor_declaration());
	while (_tokenizer.peek().type == TOK_PIPE) {
		_tokenizer.next();
		res->children.push_back(parse_constructor_declaration());
	}

	return res;
}

AST* Parser::parse_variable_declaration(void) {
	assert_type(TOK_VAR);
	_tokenizer.next();

	AST* res = new AST();
	res->kind = AST_VARIABLE_DECLARATION;

	assert_type(TOK_LOWERID);
	res->token = _tokenizer.peek(); // Nombre de la variable
	_tokenizer.next();
		
	if (_tokenizer.peek().type == TOK_COLON) {
		_tokenizer.next();
		res->children.push_back(parse_type()); // Tipo de la variable
	} else {
		res->children.push_back(NULL);
	}

	assert_type(TOK_DEF);
	_tokenizer.next();

	res->children.push_back(parse_expression()); // Valor de la variable

	return res;
}

AST* Parser::parse_let(void) {
	assert_type(TOK_LET);
	_tokenizer.next();

	AST* res = new AST();
	res->kind = AST_LET;

	while (_tokenizer.peek().type != TOK_IN) {
		if (_tokenizer.peek().type == TOK_TYPE) {
			res->children.push_back(parse_type_declaration());
		} else if (_tokenizer.peek().type == TOK_VAR) {
			res->children.push_back(parse_variable_declaration());
		} else {
			cerr << "Se esperaba una declaracion de tipo ";
			cerr << "o una declaracion de variable" << endl;
			cerr << "Se recibio token " << _tokenizer.peek().type << endl;
			exit(1);
		}	
	}

	assert_type(TOK_IN);
	_tokenizer.next();

	res->children.push_back(parse_expression()); // Cuerpo del let

	return res;
}

AST* Parser::parse_case(void) {
	assert_type(TOK_CASE);
	_tokenizer.next();
	
	AST* res = new AST();
	res->kind = AST_CASE;
	res->children.push_back(parse_expression());

	assert_type(TOK_OF);
	_tokenizer.next();

	while (_tokenizer.peek().type == TOK_PIPE) {
		_tokenizer.next();
		
		// Patron
		AST* pattern = parse_expression();
		if (!is_pattern(pattern)) {
			cerr << "Error en el case:" << endl;
			pattern->show(cerr, 1);
			cerr << endl;
			cerr << "no es un patron." << endl;
			exit(1);
		}
		res->children.push_back(pattern);

		assert_type(TOK_FAT_ARROW);
		_tokenizer.next();

		// Resultado
		res->children.push_back(parse_expression());
	}
	
	assert_type(TOK_END);
	_tokenizer.next();

	return res;
}

AST* Parser::parse_application(void) {
	AST* res = parse_atom();
	while (!is_application_terminator(_tokenizer.peek().type)) {
		AST* ast = new AST();
		ast->kind = AST_APP;
		ast->children.push_back(res);
		ast->children.push_back(parse_atom());
		res = ast;
	}
	return res;
}

bool Parser::is_operator_at_level(TokenType t, unsigned int level) {
	unsigned int j;
	for (j = 0; j < _precedence_table[level].size(); j++) {
		if (_precedence_table[level][j] == t) {
			return true;
		}
	}
	return false;
}

AST* Parser::parse_operators(unsigned int level) {
	if (level == _precedence_table.size()) {
		return parse_application();	
	}

	if (_arity_table[level] == OPERATOR_UNARY) {
		if (is_operator_at_level(_tokenizer.peek().type, level)) {
			AST* res = new AST();
			res->kind = AST_UNOP;
			res->token = _tokenizer.peek();
			_tokenizer.next();

			res->children.push_back(parse_operators(level));
			return res;
		} else {
			return parse_operators(level + 1);
		}
	} else {
		AST* res = parse_operators(level + 1);
		while (is_operator_at_level(_tokenizer.peek().type, level)) {
			AST* aux = new AST();
			aux->kind = AST_BINOP;
			aux->token = _tokenizer.peek();
			_tokenizer.next();

			aux->children.push_back(res);
			aux->children.push_back(parse_operators(level + 1));
			res = aux;
		}
		return res;
	}
}

AST* Parser::parse_expression(void) {
	Token t = _tokenizer.peek();

	if (t.type == TOK_CASE) {
		return parse_case();
	} else if (t.type == TOK_FUN) {
		return parse_fun();
	} else if (t.type == TOK_LET) {
		return parse_let();
	} else {
		return parse_operators(0);
	}
}

#include <fstream>
#include <iostream>

int main() {
	ifstream input("test.txt");
	Tokenizer tok(input);

	/*
	while (!tok.eof()) {
		Token t = tok.peek();
		cout << "lei <type=" << t.type << ", value=[[ " << t.value << " ]]>" << endl;
		tok.next();
	}
	Token t = tok.peek();
	cout << "lei <type=" << t.type << ", value=[[ " << t.value << " ]]>" << endl;
	*/
	
	Parser p(tok);
	p.parse_expression()->show(cout);
	cout << endl;

	return 0;
}

