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

ostream& operator<<(ostream& os, AST* ast) {
	ast->show(os);
	return os;
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

AST* make_metavar(const string& name) {
	AST* res = new AST();
	res->token.value = "?" + name + "@<" + pointer_to_string(res) + ">";
	res->kind = AST_TYPE_METAVAR;
	res->children.push_back(NULL);
	return res;
}

AST* make_type_arrow(AST* param_type, AST* ret_type) {
	AST* res = new AST();
	res->kind = AST_TYPE_ARROW;
	res->children.push_back(param_type);
	res->children.push_back(ret_type);
	return res;
}

