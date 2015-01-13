#include <cstdlib>
#include <vector>
#include "tokenizer.h"

using namespace std;

typedef enum {
	AST_CONSTRUCTOR,
	AST_VARIABLE,
	AST_CONSTANT,
	AST_APP,
	AST_CASE,
	AST_FUN,
} ASTKind;

struct AST {
	ASTKind kind;
	Token token;
	vector<AST*> children;

	void show(ostream& os, int level) const;
};

void AST::show(ostream& os, int level) const {
	int i;
	for (i = 0; i < level; i++) {
		os << "  ";
	}
	os << "(" << kind << " " << token.type << " " << token.value;
	
	for (i = 0; i < children.size(); i++) {
		os << endl;
		children[i]->show(os, level + 1);
	}

	os << ")"; 

}

class Parser {
	public:
		Parser(Tokenizer& t);
		
		AST* parse_atom(void);
		AST* parse_expression(void);
	private:
		Tokenizer& _tokenizer;
};

Parser::Parser(Tokenizer& t) : _tokenizer(t) {
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

bool is_terminator(TokenType t) {
	return		t == TOK_EOF
		||	t == TOK_RPAREN;
}

AST* Parser::parse_expression(void) {
	Token t = _tokenizer.peek();

	if (t.type == TOK_CASE) {
		AST* res = new AST();
		cout << "es un case" << endl;
		//TODO
	} else {
		vector<AST*> exprs;
		exprs.push_back(parse_atom());
		while (!is_terminator(_tokenizer.peek().type)) {
			exprs.push_back(parse_atom());
		}
	
		AST* res = exprs[0];
		int i;
		for (i = 1; i < exprs.size(); i++) {
			AST* ast = new AST();
			ast->kind = AST_APP;
			ast->children.push_back(res);
			ast->children.push_back(exprs[i]);
			res = ast;
		}
		return res;
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
	p.parse_expression()->show(cout, 0);
	cout << endl;

	return 0;
}
