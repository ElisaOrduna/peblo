#include <cassert>

#include "unify.h"

AST* representant(AST* t) {
	AST* r = t;
	while (r->kind == AST_TYPE_METAVAR && r->children[0] != NULL) {
		r = r->children[0];
	}
	while (t->kind == AST_TYPE_METAVAR && t->children[0] != NULL) {
		AST* tmp = t->children[0];
		t->children[0] = r;
		t = tmp;
	}
	return r;
}

bool occurs(AST* x, AST* ast) {
	if (ast == NULL) {
		return false;
	}

	bool res = (x == ast);	
	for (unsigned int i = 0; i < ast->children.size(); i++) {
		res = res || occurs(x, ast->children[i]);
	}
	return res;
}

bool unify(AST* t1, AST* t2) {
	t1 = representant(t1);
	t2 = representant(t2);

	assert(t1->kind == AST_TYPE_METAVAR || t1->kind == AST_TYPE_ARROW || t1->kind == AST_TYPE_APP);
	assert(t2->kind == AST_TYPE_METAVAR || t2->kind == AST_TYPE_ARROW || t2->kind == AST_TYPE_APP);

	if (t1 == t2) {
		return true;
	} else if (t1->kind == AST_TYPE_METAVAR) {
		if (occurs(t1, t2)) {
			return false;
		}
		t1->children[0] = t2;
		return true;
	} else if (t2->kind == AST_TYPE_METAVAR) {
		if (occurs(t2, t1)) {
			return false;
		}
		t2->children[0] = t1;
		return true;
	} else if (t1->kind == AST_TYPE_ARROW && t2->kind == AST_TYPE_ARROW) {
		return unify(t1->children[AST_TYPE_ARROW_DOMAIN], t2->children[AST_TYPE_ARROW_DOMAIN])
		    && unify(t1->children[AST_TYPE_ARROW_CODOMAIN], t2->children[AST_TYPE_ARROW_CODOMAIN]);
	} else if (t1->kind == AST_TYPE_APP && t2->kind == AST_TYPE_APP) {
		assert(t1->children[0]->kind == AST_CONSTRUCTOR_SYMBOL);
		assert(t2->children[0]->kind == AST_CONSTRUCTOR_SYMBOL);
		
		if (t1->children.size() != t2->children.size()) {
			return false;
		}

		bool res = t1->children[0] == t2->children[0];
		for (unsigned int i = 1; i < t1->children.size(); i++) {
			res = res && unify(t1->children[i], t2->children[i]);
		}

		return res;
	} else {
		return false;
	}
}
