#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include <map>
#include <vector>
#include <set>
#include <cassert>

template<typename Key, typename Value>
class Environment {
	public:
		Environment();
		bool defined(const Key& k) const;
		bool defined_in_current_scope(const Key& k) const;
		const Value& current_value(const Key& k) const;

		void begin_scope(void);
		void push_def(const Key& k, const Value& v);
		void end_scope(void);
	private:
		std::map<Key, std::vector<Value> > _env;
		std::vector<std::set<Key> > _scopes;
		void pop_def(const Key& k);
};

template<typename Key, typename Value>
Environment<Key, Value>::Environment() {
	_scopes.push_back(std::set<Key>());
}

template<typename Key, typename Value>
void Environment<Key, Value>::begin_scope(void) {
	_scopes.push_back(std::set<Key>());
}

template<typename Key, typename Value>
void Environment<Key, Value>::end_scope(void) {
	std::set<Key>& current_scope = _scopes[_scopes.size() - 1];

	typename std::set<Key>::iterator it;
	for (it = current_scope.begin(); it != current_scope.end(); ++it) {
		pop_def(*it);
	}
	_scopes.pop_back();
}

template<typename Key, typename Value>
bool Environment<Key, Value>::defined(const Key& k) const {
	return _env.find(k) != _env.end();
}

template<typename Key, typename Value>
const Value& Environment<Key, Value>::current_value(const Key& k) const {
	assert(defined(k));
	const std::vector<Value>& vec = _env.at(k);
	return vec[vec.size() - 1];
}

template<typename Key, typename Value>
bool Environment<Key, Value>::defined_in_current_scope(const Key& k) const {
	const std::set<Key>& current_scope = _scopes[_scopes.size() - 1];
	return current_scope.find(k) != current_scope.end();
}

template<typename Key, typename Value>
void Environment<Key, Value>::push_def(const Key& k, const Value& v) {
	assert(!defined_in_current_scope(k));
	_scopes[_scopes.size() - 1].insert(k);
	if (!defined(k)) {
		_env[k] = std::vector<Value>();
	}
	_env[k].push_back(v);
}

template<typename Key, typename Value>
void Environment<Key, Value>::pop_def(const Key& k) {
	std::vector<Value>& vec = _env[k];
	vec.pop_back();
	if (vec.size() == 0) {
		_env.erase(k);	
	}
}

#endif
