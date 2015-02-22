#include <fstream>
#include <iostream>

#include "tokenizer.h"
#include "parser.h"
#include "rename.h"
#include "ast.h"
#include "unify.h"
#include "infer.h"

using namespace std;

void test_unify() {
	AST* a = make_metavar("a");
	AST* b = make_metavar("b");

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

	AST* alfa = make_metavar("alfa");
	AST* beta = make_metavar("beta");
	AST* gamma = make_metavar("gamma");
	AST* delta = make_metavar("delta");
	AST* epsilon = make_metavar("epsilon");

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

	//
	AST* X = make_metavar("X");
	AST* XflBool = new AST();
	XflBool->kind = AST_TYPE_ARROW;
	XflBool->children.push_back(X);
	XflBool->children.push_back(Bool);
	AST* Bool_XflBool = new AST();
	Bool_XflBool->kind = AST_TYPE_APP;
	Bool_XflBool->children.push_back(Bool);
	Bool_XflBool->children.push_back(XflBool);
	cout << "X === Bool (X -> Bool): " << unify(X, Bool_XflBool) << endl;
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
	cout << ast << endl;

	Renamer renamer;
	ast = renamer.rename(ast);
	cout << "----------------------------------------AST renombrado" << endl;
	cout << ast << endl;

	//cout << "----------------------------------------unificacion" << endl;
	//test_unify();

	TypeInferrer inferrer;
	cout << "----------------------------------------Inferencia de tipos" << endl;
	cout << inferrer.infer(ast) << endl;

	return 0;
}


