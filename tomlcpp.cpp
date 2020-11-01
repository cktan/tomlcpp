#include <memory>
#include <vector>
#include "tomlcpp.hpp"
#include "toml.h"
#include <string.h>

using namespace toml;
using std::string;

/**
 *  Keep track of memory to be freed when all references
 *  to the tree returned by toml::parse is no longer reachable.
 */
struct toml::Backing {
	char* ptr;
	toml_table_t* root;
	Backing(char* p) : ptr(p) {}
	~Backing() {
		free(ptr);
		if (root) toml_free(root);
	}
};

string* Value::toString() const
{
	char* s;
	if (toml_rtos(m_raw, &s)) return 0;
	strval = s;
	free(s);
	return &strval;
}

bool* Value::toBool() const
{
	int b;
	if (toml_rtob(m_raw, &b)) return 0;
	boolval = !!b;
	return &boolval;
}

int64_t* Value::toInt() const
{
	if (toml_rtoi(m_raw, &intval)) return 0;
	return &intval;
}

double* Value::toDouble() const
{
	if (toml_rtod(m_raw, &doubleval)) return 0;
	return &doubleval;
}


Timestamp* Value::toTimestamp() const
{
	toml_timestamp_t ts;
	if (toml_rtots(m_raw, &ts)) return 0;
	timestampval.year = (ts.year ? *ts.year : -1);
	timestampval.month = (ts.month ? *ts.month : -1);
	timestampval.day = (ts.day ? *ts.day : -1);
	timestampval.hour = (ts.hour ? *ts.hour : -1);
	timestampval.second = (ts.second ? *ts.second : -1);
	timestampval.millisec = (ts.millisec ? *ts.millisec : -1);
	timestampval.z = ts.z ? string(ts.z) : "";
	return &timestampval;
}


std::unique_ptr<Value> Table::getValue(const string& key) const
{
	toml_raw_t r = toml_raw_in(m_table, key.c_str());
	if (!r)
		return 0;
	
	auto ret = std::make_unique<Value>(r, m_backing);
	return ret;
}


std::unique_ptr<Array> Table::getArray(const string& key) const
{
	toml_array_t* a = toml_array_in(m_table, key.c_str());
	if (!a)
		return 0;

	auto ret = std::make_unique<Array>(a, m_backing);
	return ret;
}

std::unique_ptr<Table> Table::getTable(const string& key) const
{
	toml_table_t* t = toml_table_in(m_table, key.c_str());
	if (!t)
		return 0;

	auto ret = std::make_unique<Table>(t, m_backing);
	return ret;
}

std::vector<string> Table::keys() const
{
	std::vector<string> vec;
	for (int i = 0; ; i++) {
		const char* k = toml_key_in(m_table, i);
		if (!k) break;
		vec.push_back(k);
	}
	return vec;
}


char Array::kind() const
{
	return toml_array_kind(m_array);
}

char Array::type() const
{
	return toml_array_type(m_array);
}

std::unique_ptr<Value> Array::getValue(int idx) const
{
	toml_raw_t r = toml_raw_at(m_array, idx);
	if (!r)
		return 0;
	
	auto ret = std::make_unique<Value>(r, m_backing);
	return ret;
}

std::unique_ptr<Array> Array::getArray(int idx) const
{
	toml_array_t* a = toml_array_at(m_array, idx);
	if (!a)
		return 0;

	auto ret = std::make_unique<Array>(a, m_backing);
	return ret;
}

std::unique_ptr<Table> Array::getTable(int idx) const
{
	toml_table_t* t = toml_table_at(m_array, idx);
	if (!t)
		return 0;

	auto ret = std::make_unique<Table>(t, m_backing);
	return ret;
}


toml::ParserResult toml::parse(const string& conf)
{
	toml::ParserResult ret;
	char errbuf[200];
	char* s = strdup(conf.c_str());
	if (!s) {
		ret.errmsg = "out of memory";
		return ret;
	}
	auto backing = std::make_shared<Backing>(s);
	
	toml_table_t* t = toml_parse(s, errbuf, sizeof(errbuf));
	if (t) {
		ret.table = std::make_shared<Table>(t, backing);
		backing->root = t;
	} else {
		ret.errmsg = (*errbuf) ? string(errbuf) : "unknown error";
	}
	return ret;
}
