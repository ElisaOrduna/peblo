#include <sstream>
#include "ast.h"

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

string pointer_to_string(void* ptr) {
	stringstream ss;
	ss << ptr;
	return ss.str();
}

AST* make_symbol(const string& name, ASTKind symbol_kind) {
	AST* res = new AST();
	res->kind = symbol_kind;
	res->token.value = name + "@<" + pointer_to_string(res) + ">";
	return res;
}

