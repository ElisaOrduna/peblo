#include <fstream>
#include <iostream>

#include "tokenizer.h"
#include "parser.h"
#include "rename.h"

using namespace std;

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
	return 0;
}

