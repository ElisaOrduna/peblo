#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include <map>
#include <vector>

template<typename Key, typename Value>
class Environment {
	public:
		bool defined(const Key& k) const;
		const Value& current_value(const Key& k) const;
		void push_def(const Key& k, const Value& v);
		void pop_def(const Key& k);
	private:
		std::map<Key, std::vector<Value> > _env;
};

template<typename Key, typename Value>
bool Environment<Key, Value>::defined(const Key& k) const {
	return _env.find(k) != _env.end();
}

template<typename Key, typename Value>
const Value& Environment<Key, Value>::current_value(const Key& k) const {
	const std::vector<Value>& vec = _env.at(k);
	return vec[vec.size() - 1];
}

template<typename Key, typename Value>
void Environment<Key, Value>::push_def(const Key& k, const Value& v) {
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
