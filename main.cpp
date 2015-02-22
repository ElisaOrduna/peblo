#include <fstream>
#include <iostream>

#include "tokenizer.h"
#include "parser.h"
#include "rename.h"
#include "unify.h"

using namespace std;

AST* mk_metavar(const string& name) {
	AST* a = new AST();
	a->token.value = "?" + name;
	a->kind = AST_TYPE_METAVAR;
	a->children.push_back(NULL);
	return a;
}

void test_unify() {
	AST* a = mk_metavar("a");
	AST* b = mk_metavar("b");

	AST* Int = new AST();
	Int->kind = AST_TYPE_APP;
	Int->children.push_back(make_symbol("Int", AST_CONSTRUCTOR_SYMBOL));

	AST* Bool = new AST();
	Bool->kind = AST_TYPE_APP;
	Bool->children.push_back(make_symbol("Bool", AST_CONSTRUCTOR_SYMBOL));

	//
	cout << "a/a: " << unify(a, a) << endl;
	cout << a << endl;
	//
	cout << "a/b: " << unify(a, b) << endl;
	cout << a << endl;
	cout << b << endl;
	//
	cout << "a/Int: " << unify(a, Int) << endl;
	cout << a << endl;
	cout << b << endl;
	//
	cout << "Int/a: " << unify(Int, a) << endl;
	cout << a << endl;
	cout << b << endl;
	//
	cout << "Int/Bool: " << unify(Int, Bool) << endl;
	//
	cout << "Bool/a: " << unify(Bool, a) << endl;
	cout << a << endl;
	cout << b << endl;
	//

	AST* alfa = mk_metavar("alfa");
	AST* beta = mk_metavar("beta");
	AST* gamma = mk_metavar("gamma");
	AST* delta = mk_metavar("delta");
	AST* epsilon = mk_metavar("epsilon");

	AST* beta_fl_beta = new AST();
	beta_fl_beta->kind = AST_TYPE_ARROW;
	beta_fl_beta->children.push_back(beta);
	beta_fl_beta->children.push_back(beta);

	unify(alfa, beta_fl_beta);
	unify(gamma, delta);
	unify(epsilon, Bool);

	AST* delta_fl_bool = new AST();
	delta_fl_bool->kind = AST_TYPE_ARROW;
	delta_fl_bool->children.push_back(gamma);
	delta_fl_bool->children.push_back(epsilon);

	cout << "a(->(b,b)) === ->(a(d),e(B)): " << unify(beta_fl_beta, delta_fl_bool) << endl;
	cout << alfa << endl;
	cout << beta << endl;
	cout << gamma << endl;
	cout << delta << endl;
	cout << epsilon << endl;
}

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
	AST* ast = p.parse_expression();

	cout << "----------------------------------------AST original" << endl;
	ast->show(cout);
	cout << endl;

	Renamer renamer;
	ast = renamer.rename(ast);
	cout << "----------------------------------------AST renombrado" << endl;
	ast->show(cout);
	cout << endl;

	cout << "----------------------------------------unificacion" << endl;
	test_unify();

	return 0;
}


